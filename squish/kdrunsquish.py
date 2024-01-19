#!/usr/bin/env python3

#
# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

"""
This script runs squish tests in parallel via the Qt offscreen QPA
"""

import signal
import time
import sys
import threading
import subprocess
import os
import argparse
import json
import multiprocessing
import re
import asyncio
import io
import platform
import psutil

# pylint: disable=invalid-name,bare-except

TESTS_JSON_FILENAME = 'tests.json'

INSTRUCTIONS = '''
This script runs squish tests in parallel via the Qt offscreen QPA

You need to list your tests inside a %s file. See kdtoolbox/squish/tests.json
for an example.

The top level attributes are:
  - 'aut'               : Mandatory, specifies the name of the binary to be tested, known as the AUT in squish lingo.
  - 'startScript'       : Optional, specifies the name of the script that starts the AUT. In case we do not start the AUT directly

The supported attributes for each test %s are:

  - 'name'              : Mandatory, the name of the test. Will be passed to squishrunner's --testcase argument.
  - 'suite'             : Mandatory, the name of the suite. Will be passed to squishrunner's --testsuite argument.
  - 'categories'        : Optional, list of categories. These are user defined and have no meaning for the script, other than allowing to run just
                          the tests that belong to a certain category.
  - 'disabled'          : Optional, defaults to false. If true, the test won't be executed.
  - 'supports_offscreen': Optional, defaults to true. If false, the test won't be executed via offscreen QPA, only if --native is passed.
  - 'failure_expected'  : Optional, defaults to false. If true, then it's expected that the test returns non-0 for this script to return 0.


Each test will be assigned a sequential integer ID, which will be passed to squishrunner's --scriptargs argument. You can use this ID
in your tests so you can use different ports and config directories by prefixing the ID.


Example usage:

  First time usage, aut needs to be specified (On following runs it reads the auth path from server.ini, this authPAth will be optional):
    $ kdrunsquish.py -j 10 --autPath /my/bindir/ mytests/

  Run with 10 parallel tests:
    $ kdrunsquish.py -j 10 mytests/

  List all tests:
    $ kdrunsquish.py -l

  See complete help with --help
''' % (TESTS_JSON_FILENAME, TESTS_JSON_FILENAME)

s_numCPUs = multiprocessing.cpu_count()
s_verbose = False
s_continuous_output = False
s_isOffscreen = False
s_resultDir = ''
s_stdoutLock = threading.Lock()
s_squishServerStartPort = 50000  # TODO: make configurable
s_maxFlakyRuns = 1
os.environ['SQUISH_NO_CAPTURE_OUTPUT'] = '1'  # fixes corrupt output by squish
s_startTime = time.time()
s_autPath = ''
s_startScriptPath = ''
s_outputFilters = []
s_allOutputTasks = []
# pylint: enable=invalid-name


def runCommandSync(cmdArgs):
    ''' Runs a command and blocks. Returns true if the command executed successfully'''
    if s_verbose:
        print("Running: " + " ".join(cmdArgs))

    try:
        return subprocess.run(cmdArgs, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    except FileNotFoundError as e:
        print("ERROR: %s probably not found in PATH! %s" % (cmdArgs[0], e))
        sys.exit(1)


def _ignoreLine(line):
    for outputFilter in s_outputFilters:
        if outputFilter.match(line):
            return True

    return False


async def _handleStdout(process, output):
    async for data in process.stdout:
        line = data.decode('utf-8')
        if not _ignoreLine(line):
            output.write(line)
            if s_continuous_output:
                print(line)


def matchPlatform(value):
    system = platform.system()
    if system == "Linux" and value.lower() in ["linux"]:
        return True
    if system == "Darwin" and value.lower() in ["macos"]:
        return True
    if system == "Windows" and value.lower() in ["windows"]:
        return True

    return False


def testPlatformFlag(value, default=False):
    if isinstance(value, bool):
        return value

    if isinstance(value, str):
        return matchPlatform(value)

    if isinstance(value, list):
        for subValue in value:
            if matchPlatform(subValue):
                return True
        return False

    return default


async def runCommandASync(cmdArgs, output, env):
    '''Starts a child process and returns immediately'''
    if s_verbose:
        print("Running: " + " ".join(cmdArgs))

    try:
        process = await asyncio.create_subprocess_exec(*cmdArgs,
                                                       stdout=asyncio.subprocess.PIPE,
                                                       stderr=asyncio.subprocess.STDOUT,
                                                       env=env)
        # Create a task to read and filter process output and stop the pipe becoming full
        s_allOutputTasks.append(asyncio.create_task(_handleStdout(process, output)))
        return process

    except FileNotFoundError as e:
        print("ERROR: %s probably not found in PATH! %s" % (cmdArgs[0], e))
        sys.exit(1)


def killProcess(proc):
    '''kills a process and its children'''
    # pylint: disable=no-member #don't complain about missing check= option
    try:
        processes = psutil.Process(proc.pid).children(recursive=True) + [proc]
    except Exception:
        return
    for p in processes:
        try:
            p.kill()
        except ProcessLookupError:
            pass
        except Exception as e:
            try:
                print("ERROR: Could not kill process %s: %s" %
                      (proc.name(), e))
            except:
                print("ERROR: Could not kill process %s: %s" % (proc.pid, e))


def killProcessByPort(name, port):
    '''Kills the process with the specified named if it's listening on the specified port'''
    try:
        try:
            connections = list(filter(lambda p: p.laddr and p.laddr[1] == port,
                                      psutil.net_connections(kind='tcp4')))
        except:
            print("ERROR: Could not list process by port %s: %s, requires root privileges" % (name, port))
            return

        if connections:
            proc = psutil.Process(connections[0].pid)
            if proc.name() == name:
                killProcess(proc)
    except Exception as e:
        print("ERROR: Could not kill process by port %s: %s" % (name, e))

# pylint: disable=too-many-instance-attributes


class SquishTest:
    '''Represents a single squish test'''

    s_nextId = 1

    def __init__(self, name, suite, testId):
        self.name = name
        self.suite = suite
        self.testId = testId
        self.disabled = False
        self.supportsOffscreen = True
        self.failureExpected = False
        self.serverProc = None
        self.runnerProc = None
        self.__maxFlakyRuns = s_maxFlakyRuns
        self.__numRuns = 0
        self.__wasSkipped = False
        self.serverStdout = io.StringIO()
        self.runnerStdout = io.StringIO()
        self.categories = []

        ''' The number of times this test failed'''
        self.numFailures = 0

        ''' The number of times this test succeeded'''
        self.numSuccesses = 0

    @staticmethod
    def fromJson(jsonTest):
        SquishTest.s_nextId += 1
        squishTest = SquishTest(
            jsonTest['name'], jsonTest['suite'], SquishTest.s_nextId)

        if 'supports_offscreen' in jsonTest:
            squishTest.supportsOffscreen = testPlatformFlag(jsonTest['supports_offscreen'], True)
        if 'disabled' in jsonTest:
            squishTest.disabled = testPlatformFlag(jsonTest['disabled'])
        if 'failure_expected' in jsonTest:
            squishTest.failureExpected = testPlatformFlag(jsonTest['failure_expected'])
        if 'categories' in jsonTest:
            squishTest.categories = jsonTest['categories']

        return squishTest

    def remainingRuns(self):
        return self.__maxFlakyRuns - self.__numRuns

    def serverPort(self):
        return str(15000 + self.testId)

    def squishserverArgs(self):
        serverArgs = ['squishserver',
                      '--port',
                      self.serverPort(),
                      '--configfile', "server.ini"]
        return serverArgs

    def squishrunnerArgs(self):
        '''Returns the arguments to be passed to the squishrunner binary'''
        return ['squishrunner',
                '--port',
                self.serverPort(),
                '--testsuite',
                self.suite,
                '--exitCodeOnFail',
                '1',
                '--abortOnFail',
                '--reportgen',
                'stdout',
                '--testcase',
                self.name,
                '--scriptargs',
                str(self.testId)
                ]

    def markSkipped(self):
        self.__wasSkipped = True

    def wasSkipped(self):
        return self.__wasSkipped

    def ran(self):
        '''Returns whether this test ran (regardless of success/error)'''
        return self.__numRuns > 0

    def wasFlaky(self):
        '''Returns whether there was a mix of success and failure'''
        return self.numSuccesses > 0 and self.numFailures > 0

    def run(self, env):
        killProcessByPort('_squishserver', int(self.serverPort()))

        loop = asyncio.new_event_loop()
        returncode = loop.run_until_complete(self._runAsync(env))
        loop.close()

        return returncode

    async def _runAsync(self, env):

        if self.remainingRuns() <= 0:
            # Doesn't happen
            print("ERROR: Test %s was already executed" % self.name)
            sys.exit(1)

        self.__numRuns = self.__numRuns + 1

        self.serverProc = await runCommandASync(self.squishserverArgs(), self.serverStdout, env)
        self.runnerProc = await runCommandASync(self.squishrunnerArgs(), self.runnerStdout, env)

        returncode = await self.runnerProc.wait()

        serverPort = int(self.serverPort())
        if runCommandSync(['squishserver', '--stop', '--port', str(serverPort)]).returncode != 0:
            print("ERROR: Could not stop the squishserver, port: %s" % serverPort)
            sys.exit(1)

        await self.serverProc.wait()

        # Wait on all background tasks to finish reading
        await asyncio.gather(*asyncio.all_tasks(asyncio.get_running_loop()) - {asyncio.current_task()})

        # pylint: disable=global-statement,invalid-name,global-variable-not-assigned
        global s_resultDir
        if s_resultDir:
            with open('/%s/%s.out' % (s_resultDir, self.name), 'w', encoding="utf8",) as f:
                f.write('\nServer output for test %s\n' % self.name)
                f.write(self.serverStdout.getvalue())
                f.write('\nRunner output for test %s\n' % self.name)
                f.write(self.runnerStdout.getvalue())

        passedExpectedly = returncode == 0 and not self.failureExpected
        passedUnexpectedly = returncode == 0 and self.failureExpected
        failedExpectedly = returncode != 0 and self.failureExpected
        failedUnexpectedly = returncode != 0 and not self.failureExpected

        success = passedExpectedly or failedExpectedly

        # Print squish's output if needed:
        if not s_continuous_output and (s_verbose or returncode != 0):
            with s_stdoutLock:
                print(self.serverStdout.getvalue())
                print(self.runnerStdout.getvalue())

        if passedUnexpectedly:
            tag = '[XOK ]'
        elif passedExpectedly:
            tag = '[OK  ]'
        elif failedExpectedly:
            tag = '[XFAIL ]'
        elif failedUnexpectedly:
            tag = '[FAIL]'

        print('%s %s' % (tag, self.name))

        if success:
            self.numSuccesses += 1
        else:
            self.numFailures += 1

        return success

    def matchesCategories(self, categories):
        '''Returns whether this test belongs to any of the specified categories'''
        # Simply return if the 2 lists intersect
        return bool(set(self.categories) & set(categories))


class Statistics:
    '''Simple struct just to contain the result of the test run'''

    def __init__(self, squishTests) -> None:
        # self.__wasSuccessful = True currently unused
        self.__numTestsRan = 0
        self.__numTestsSkipped = 0
        self.__flakyTestNames = []
        self.__failedTestNames = []

        for squishTest in squishTests:
            if squishTest.wasSkipped():
                self.__numTestsSkipped += 1
            elif squishTest.ran():
                self.__numTestsRan += 1

                if squishTest.wasFlaky():
                    self.__flakyTestNames.append(squishTest.name)
                elif squishTest.numFailures > 0:
                    self.__failedTestNames.append(squishTest.name)

    def wasSuccessful(self):
        '''Returns whether all tests were successful'''
        return len(self.__failedTestNames) == 0

    def printStats(self):
        print("Ran %d, skipped %d, failed %d, flaky %d" %
              (self.__numTestsRan, self.__numTestsSkipped, len(self.__failedTestNames), len(self.__flakyTestNames)))
        if self.__failedTestNames:
            print("Failed tests: %s" % ','.join(self.__failedTestNames))
        if self.__flakyTestNames:
            print("Flaky tests: %s" % ','.join(self.__flakyTestNames))


# pylint: disable=no-self-use
class Platform:
    def runSingleTest(self, squishTest: SquishTest) -> bool:
        if not self.prepare():
            return False

        while squishTest.remainingRuns() > 0:  # if --maxFlakyRuns is passed, we'll execute until it passes
            success = squishTest.run(self.env())
            if success:
                break

        self.cleanup()

        # print("Finished running %s" % squishTest.name)
        return success

    def prepare(self):
        '''Called before running a test'''
        return True

    def cleanup(self):
        '''Called after running a test'''
        return True

    def env(self):
        '''Returns the environment where this test should run'''
        testenv = os.environ.copy()
        testenv.update(self.extraEnv())
        return testenv

    def extraEnv(self):
        '''Returns any additional env variables. These will be merged with current environment.'''
        return {}


class TestsRunner:
    ''' Class responsible for loading and running all tests'''

    def __init__(self) -> None:
        self.tests = []
        self.aut = ''
        self.startScript = ''
        self.globalScriptDir = ''
        self.loadTests()
        self.testsAborted = False
        self._lock = threading.Lock()
        self._abortsOnFail = False

    def enableAbortOnFail(self):
        self._abortsOnFail = True

    def loadTests(self):
        contents = ''
        try:
            with open(TESTS_JSON_FILENAME, 'r', encoding='utf8') as f:
                contents = f.read()
        except OSError as e:
            print("Failed to read %s because: %s" % (TESTS_JSON_FILENAME, e))
            sys.exit(-1)

        decoded = json.loads(contents)
        if 'globalScriptDir' in decoded:
            self.globalScriptDir = decoded['globalScriptDir']
            if not os.path.exists(self.globalScriptDir):
                sys.exit(1)

            self.globalScriptDir = os.path.abspath(self.globalScriptDir)
        if 'aut' in decoded:
            self.aut = decoded['aut']

        if 'startScript' in decoded:
            self.startScript = decoded['startScript']

        if 'env' in decoded:
            for key in decoded['env']:
                os.environ[key] = decoded['env'][key]

        if 'outputFilters' in decoded:
            outputFilters = decoded['outputFilters']
            for outputFilter in outputFilters:
                s_outputFilters.append(re.compile(outputFilter))

        jsonTests = decoded['tests']
        for jsonTest in jsonTests:
            squishTest = SquishTest.fromJson(jsonTest)

            if not squishTest.disabled:
                self.tests.append(squishTest)

        if not self.aut:
            print("ERROR: aut attribute not found in %s" % TESTS_JSON_FILENAME)
            sys.exit(1)

    @staticmethod
    def chunks(numChunks: int, requestedTestsList):
        '''Splits the list of requested tests into chunks. Returns a list of tests.
           At most numChunks lists are returned
        '''
        chunksList = [[] for x in range(numChunks)]
        i = numChunks
        for squishTest in requestedTestsList:
            i = (i + 1) % numChunks
            chunksList[i].append(squishTest)

        return chunksList

    def runInParallell(self, numJobs, requestedTestsList):
        '''Runs the requested tests, in parallel.'''

        self.createINIFiles()
        chunks = self.chunks(numJobs, requestedTestsList)

        # Discard empty lists
        chunks = list(filter(lambda chunk: len(chunk) > 0, chunks))

        if s_verbose:
            print("Running a total of %d tests split through %d threads" %
                  (len(requestedTestsList), len(chunks)))

        threads = []
        for chunk in chunks:
            t = threading.Thread(target=self.runInSequence, args=(chunk,))
            t.start()
            threads.append(t)

        if s_verbose:
            print("Waiting for threads to finish...")

        for thread in threads:
            thread.join()

        if s_verbose:
            print("All threads finished.")

    @staticmethod
    def platformForTest(squishTest: SquishTest):
        '''Returns a suitable Platform to run the specified test. Since all tests support offscreen'''
        if s_isOffscreen:
            if squishTest.supportsOffscreen:
                return (OffscreenPlatform(), '')
            if matchPlatform('Linux'):
                supported, reason = XvfbPlatform.supported()
                if supported:
                    return (XvfbPlatform(), '')
                return (None, reason)

            return (None, 'Test does not support offscreen')

        return (NativePlatform(), '')

    def runInSequence(self, requestedTestsList):
        '''Runs the requested tests in sequence.'''

        if s_verbose:
            names = list(map(lambda test: test.name, requestedTestsList))
            print("Starting Thread to run: %d tests (%s)" %
                  (len(requestedTestsList), ",".join(names)))

        for squishTest in requestedTestsList:
            with self._lock:
                if self.testsAborted:
                    return

            (plat, skipReason) = TestsRunner.platformForTest(squishTest)

            if not plat:
                print('[SKIP] %s (%s)' % (squishTest.name, skipReason))
                squishTest.markSkipped()
                continue

            testSucceeded = plat.runSingleTest(squishTest)
            if self._abortsOnFail and not testSucceeded:
                print("Aborting the whole run since a test failed...")
                with self._lock:
                    self.testsAborted = True

        if s_verbose:
            print("Finished thread for (%s)." % (",".join(names)))

    def createINIFiles(self):

        # Create ~/.squish/ver1/paths.ini - I don't see a way for it not to be global though.
        if self.globalScriptDir:
            if runCommandSync(['squishrunner',
                               '--config', 'setGlobalScriptDirs',
                               self.globalScriptDir]).returncode != 0:
                print("ERROR: Could not set globalScriptDir")
                sys.exit(1)

        # Now create a server.ini next to our tests.json. No need to use the global one.

        # If --authPath is passed, we regenerate server.ini, as squishserver reads the aut from there
        needsNewServerIni = s_autPath or not os.path.exists("server.ini")
        if not needsNewServerIni:
            return

        if not s_autPath:
            print(
                "ERROR: You need to pass --autPath <your app bin dir> on the first run or if you delete server.ini")
            sys.exit(1)

        if not os.path.exists(s_autPath) or not os.path.isdir(s_autPath):
            print(
                'ERROR: The specified autPath does not exist or is not a folder: %s' % s_autPath)
            sys.exit(1)

        commonArgs = ['squishserver', '--configfile', 'server.ini']
        if runCommandSync(commonArgs + ['--config', 'setCursorAnimation', 'off']).returncode != 0:
            print("ERROR: Could not set cursor animation off")
            sys.exit(1)

        if runCommandSync(commonArgs + ['--config', 'addAUT', self.aut, s_autPath]).returncode != 0:
            print("ERROR: Could not set AUT %s/%s" % (self.aut, s_autPath))
            sys.exit(1)

        # if a startScriptPath is mentioned - Add that path to the server.ini file
        if s_startScriptPath:
            if runCommandSync(commonArgs + ['--config', 'addAUT', self.startScript, s_startScriptPath]).returncode != 0:
                print("ERROR: Could not set AUT %s/%s" % (self.startScript, s_startScriptPath))
                sys.exit(1)

    def suiteNames(self):
        suites = map(lambda test: test.suite, self.tests)
        return list(set(suites))

    def printTests(self):
        for squishTest in self.tests:
            print(squishTest.name)

    def printSuites(self):
        for suite in self.suiteNames():
            print(suite)

    def printCategories(self):
        for cat in self.categories():
            print(cat)

    def testByName(self, name):
        for squishTest in self.tests:
            if squishTest.name == name:
                return squishTest
        return None

    def testsBySuite(self, suiteName):
        '''Returns all tests from the specified suite'''
        return list(filter(lambda test: test.suite == suiteName, self.tests))

    def testsByCategories(self, categories):
        '''Returns all tests that match any of the specified categories'''
        return list(filter(lambda test: test.matchesCategories(categories), self.tests))

    def categories(self):
        '''Returns the list of existing distinct categories'''
        result = []
        for squishTest in self.tests:
            result += squishTest.categories

        return list(set(result))  # unique

    def killChildProcesses(self):
        for squishTest in self.tests:
            if squishTest.runnerProc:
                killProcess(squishTest.runnerProc)
            if squishTest.serverProc:
                killProcess(squishTest.serverProc)


class OffscreenPlatform(Platform):
    '''Runs tests under the offscreen QPA. Supports parallelization.'''

    def extraEnv(self):
        return {'QT_QPA_PLATFORM': 'offscreen'}


class NativePlatform(Platform):
    '''Runs test sunder the native platform (windows, xcb or cocoa). Parallelization is not supported.'''

    def extraEnv(self):
        return {'QT_QPA_PLATFORM': ''}


class XvfbPlatform(Platform):
    '''Runs tests under X11 via Xvfb so parallelization is supported'''

    display = 100

    @staticmethod
    def hasXfwm4():
        try:
            subprocess.run(["xfwm4", "--version"], capture_output=True, check=False)
        except:
            print()
            sys.exit('Failed to find xfwm4. Please install it.')
        return True

    @staticmethod
    def hasXvfb():
        try:
            subprocess.run(["xvfb-run", "--help"], capture_output=True, check=False)
        except:
            sys.exit('Failed to find xvfb-run. Please install it.')
        return True

    def __init__(self):
        self.xvfbPid = None

    def _runXvfb(self):
        XvfbPlatform.display = XvfbPlatform.display + 1
        cmd = ['xvfb-run', '-n', str(XvfbPlatform.display), '-s',
               "-ac -screen 0 1920x1080x24", 'dbus-run-session', 'xfwm4']
        # pylint: disable=subprocess-popen-preexec-fn
        try:
            with subprocess.Popen(cmd, preexec_fn=os.setsid,
                                  stdout=subprocess.DEVNULL,
                                  stderr=subprocess.DEVNULL) as xvfbProcess:
                self.xvfbPid = xvfbProcess.pid
                return True
        except:
            sys.exit('Failed to run: {}'.format(' '.join(cmd)))

    @staticmethod
    def supported():
        '''Returns whether running under xvfb is supported'''
        if not matchPlatform('Linux'):
            return (False, 'Virtual X11 only supported on Linux')
        if not XvfbPlatform.hasXfwm4():
            return (False, 'Could not find xfwm4. Please install it.')
        if not XvfbPlatform.hasXvfb():
            return (False, 'Could not find xvfb-run. Please install it.')

        return (True, '')

    def extraEnv(self):
        return {'QT_QPA_PLATFORM': 'xcb', 'DISPLAY': ':{}'.format(self.display)}

    def prepare(self):
        '''Called before running a test. Starts xvfb'''
        return self._runXvfb()

    def cleanup(self):
        '''Called after running a test. Kills xcfb.'''
        if self.xvfbPid:
            try:
                os.killpg(os.getpgid(self.xvfbPid), signal.SIGTERM)
            except:
                pass


parser = argparse.ArgumentParser(description=INSTRUCTIONS)
parser.add_argument('-v', '--verbose', action='store_true', required=False,
                    help='Verbose mode')
parser.add_argument('-l', '--list', action='store_true', required=False,
                    help='Lists all tests')
parser.add_argument('-t', '--tests', required=False,
                    help='Comma separated list of test names to run')
parser.add_argument('-s', '--suites', required=False,
                    help='Comma separated list of test suites to run')
parser.add_argument('-c', '--categories', required=False,
                    help='Comma separated list of test categories to run')
parser.add_argument('--native', action='store_true', required=False,
                    help='Uses the native QPA instead of offscreen')
parser.add_argument('-o', '--outputdir', required=False,
                    help='Directory to store the tests output.')
parser.add_argument('-a', '--autPath', required=False,
                    help='Directory containing the AUT. Required the first time only, '
                          'since it will be written to server.ini')
parser.add_argument('-start', '--startScriptPath', required=False,
                    help='Directory containing the script that starts the AUT '
                    '(see --startScript). Required if needed the first time only, '
                    'since it will be written to server.ini')
parser.add_argument('squishdir', metavar='<Squish Directory>', nargs=1,
                    help='squish directory containing your tests and the %s file' % TESTS_JSON_FILENAME)
parser.add_argument('-j', '--jobs', required=False, type=int,
                    help='Number of squishrunner sessions to be run in parallel')
parser.add_argument('--maxFlakyRuns', required=False, type=int, default=1,
                    help='Runs a test at most N times until it passes.')
parser.add_argument('--continuousOutput', action='store_true', required=False,
                    help='Print squish runner and server output directly to std out as '
                    'tests are run. WARNING if multiple tests are run in parallel '
                    'then their outputs will be interleaved')

# Not to be confused with squishrunner's abortOnFail, that one we always want.
parser.add_argument('--abortOnFail', action='store_true', required=False, default=False,
                    help='aborts the whole script once the 1st test fails')

args = parser.parse_args()

s_verbose = args.verbose
s_continuous_output = args.continuousOutput

if args.autPath:
    s_autPath = os.path.abspath(args.autPath)

if args.startScriptPath:
    s_startScriptPath = os.path.abspath(args.startScriptPath)

if s_verbose:
    print("cd %s" % args.squishdir[0])

os.chdir(args.squishdir[0])

if not os.path.exists(TESTS_JSON_FILENAME):
    print("ERROR: Could not find %s!" % TESTS_JSON_FILENAME)
    sys.exit(1)

if args.jobs and args.jobs > 1 and args.native:
    print("ERROR: Native QPA only supports 1 job at a time")
    sys.exit(1)

if args.maxFlakyRuns:
    s_maxFlakyRuns = args.maxFlakyRuns
    if args.maxFlakyRuns < 1:
        print("ERROR: maxFlakyRuns should be bigger than 0 or omitted")
        sys.exit(1)

s_isOffscreen = not args.native
testsRunner = TestsRunner()

if args.list:
    print("Tests:")
    testsRunner.printTests()
    print("\nSuites:")
    testsRunner.printSuites()
    print("\nCategories:")
    testsRunner.printCategories()
    sys.exit(0)

if args.abortOnFail:
    testsRunner.enableAbortOnFail()

if args.outputdir:
    s_resultDir = args.outputdir
    if not os.path.isdir(args.outputdir):
        print("Did not find directory %s" % args.outputdir)
        sys.exit(1)

requestedTests = []
if args.tests:
    requestedTestNames = args.tests.split(",")
    for requestedTestName in requestedTestNames:
        test = testsRunner.testByName(requestedTestName)
        if not test:
            print(
                f"Unknown test {requestedTestName}. Run with -l to see a list of tests.")
            sys.exit(-1)

        requestedTests.append(test)

if args.suites:
    requestedSuiteNames = args.suites.split(",")
    for requestedSuiteName in requestedSuiteNames:
        requestedTests += testsRunner.testsBySuite(requestedSuiteName)

if args.categories:
    tests = testsRunner.testsByCategories(args.categories.split(","))
    if not tests:
        print("No tests matching the specified categories. Run with -l to see a list of tests.")
        sys.exit(-1)
    requestedTests += tests

if not args.tests and not args.suites and not args.categories:
    # Test everything
    requestedTests = testsRunner.tests


if args.jobs:
    num_jobs = args.jobs
else:
    # When running on Native, we don't support parallelism
    num_jobs = s_numCPUs if s_isOffscreen else 1

try:
    testsRunner.runInParallell(num_jobs, requestedTests)
except KeyboardInterrupt:
    print("Interrupted...")
    testsRunner.killChildProcesses()
    sys.exit(1)

results = Statistics(requestedTests)
results.printStats()

print("Took %s seconds" % (int(time.time() - s_startTime)))

if results.wasSuccessful():
    print("Success!")

testsRunner.killChildProcesses()

sys.exit(0 if results.wasSuccessful() else 1)
