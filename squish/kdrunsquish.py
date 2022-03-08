#!/usr/bin/env python3

# MIT License

# Copyright (C) 2022 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE

import time
import sys
import threading
import subprocess
import os
import argparse
import json
import multiprocessing
import psutil
import asyncio
import re

TESTS_JSON_FILENAME = 'tests.json'

INSTRUCTIONS = '''
This script runs squish tests in parallel via the Qt offscreen QPA

You need to list your tests inside a %s file. See kdtoolbox/squish/tests.json
for an example.

The top level attributes are:
  - 'aut'               : Mandatory, specifies the name of the binary to be tested, known as the AUT in squish lingo.

The supported attributes for each test %s are:

  - 'name'              : Mandatory, the name of the test. Will be passed to squishrunner's --testcase argument.
  - 'suite'             : Mandatory, the name of the suite. Will be passed to squishrunner's --testsuite argument.
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

s_num_cpus = multiprocessing.cpu_count()
s_verbose = False
s_nextId = 1
s_isOffscreen = False
s_resultdir = ''
s_stdoutLock = threading.Lock()
s_squishServerStartPort = 50000  # TODO: make configurable
s_maxFlakyRuns = 1
os.environ['SQUISH_NO_CAPTURE_OUTPUT'] = '1'  # fixes corrupt output by squish
s_startTime = time.time()
s_autPath = ''
s_outputFilers = []

def run_command_sync(args):
    ''' Runs a command and blocks. Returns true if the command executed successfully'''
    if s_verbose:
        print("Running: " + " ".join(args))

    try:
        return subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except FileNotFoundError as e:
        print("ERROR: %s probably not found in PATH! %s" % (args[0], e))
        sys.exit(1)


async def run_command_async(args):
    '''Starts a child process and returns immediately'''
    if s_verbose:
        print("Running: " + " ".join(args))

    try:
        return await asyncio.create_subprocess_exec(*args, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT)
    except FileNotFoundError as e:
        print("ERROR: %s probably not found in PATH! %s" % (args[0], e))
        sys.exit(1)


def kill_process(proc):
    '''kills a process and its children'''

    try:
        processes = psutil.Process(proc.pid).children(recursive=True) + [proc]
    except Exception as e:
        return

    for p in processes:
        try:
            p.kill()
        except Exception as e:
            print("ERROR: Could not kill process %s: %s" % (p.name, e))

def ignore_line(line):
    for filter in s_outputFilers:
        if filter.match(line):
            return True

    return False

def filter_output(output):
    return "\n".join([line for line in output.splitlines() if not ignore_line(line)])

class SquishTest:
    '''Represents a single squish test'''

    def __init__(self, name, suite, id):
        self.name = name
        self.suite = suite
        self.id = id
        self.disabled = False
        self.supportsOffscreen = True
        self.failure_expected = False
        self.serverProc = None
        self.runnerProc = None
        self.__maxFlakyRuns = s_maxFlakyRuns
        self.__numRuns = 0
        self.__wasSkipped = False
        self.serverStdout = None
        self.runnerStdout = None

        ''' The number of times this test failed'''
        self.numFailures = 0

        ''' The number of times this test succeeded'''
        self.numSuccesses = 0

    @staticmethod
    def fromJson(json):
        global s_nextId
        s_nextId += 1
        test = SquishTest(json['name'], json['suite'], s_nextId)

        if 'supports_offscreen' in json:
            test.supportsOffscreen = json['supports_offscreen']
        if 'disabled' in json:
            test.disabled = json['disabled']
        if 'failure_expected' in json:
            test.failure_expected = json['failure_expected']

        return test

    def remainingRuns(self):
        return self.__maxFlakyRuns - self.__numRuns

    def serverPort(self):
        return str(15000 + self.id)

    def squishserver_args(self):
        args = ['squishserver',
                '--port',
                self.serverPort(),
                '--configfile', "server.ini"]
        return args

    def squishrunner_args(self):
        '''Returns the arguments to be passed to the squishrunner binary'''
        return ['squishrunner',
                '--port',
                self.serverPort(),
                '--testsuite',
                self.suite,
                '--exitCodeOnFail',
                '1',
                '--reportgen',
                'stdout',
                '--testcase',
                self.name,
                '--scriptargs',
                str(self.id)
                ]

    def markSkipped(self):
        self.__wasSkipped = True

    def wasSkipped(self):
        return self.__wasSkipped

    def wasFlaky(self):
        '''Returns whether there was a mix of success and failure'''
        return self.numSuccesses > 0 and self.numFailures > 0

    def run(self):
        return asyncio.run(self._runAsync())

    async def _runAsync(self):

        if self.remainingRuns() <= 0:
            # Doesn't happen
            print("ERROR: Test %s was already executed" % self.name)
            sys.exit(1)
            return

        self.__numRuns = self.__numRuns + 1

        self.serverProc = await run_command_async(self.squishserver_args())
        self.runnerProc = await run_command_async(self.squishrunner_args())

        self.runnerStdout, _ = await self.runnerProc.communicate()

        returncode = self.runnerProc.returncode

        kill_process(self.serverProc)

        self.serverStdout, _ = await self.serverProc.communicate()

        self.runnerStdout = filter_output(self.runnerStdout.decode('utf-8'))
        self.serverStdout = filter_output(self.serverStdout.decode('utf-8'))

        global s_resultdir
        if s_resultdir:
            f = open('/%s/%s.out' % (s_resultdir, self.name), 'wb')
            f.write(str.encode('\nServer output for test %s\n' % self.name))
            f.write(str.encode(self.serverStdout))
            f.write(str.encode('\nRunner output for test %s\n' % self.name))
            f.write(str.encode(self.runnerStdout))
            f.close()

        passed_expectedly = returncode == 0 and not self.failure_expected
        passed_unexpectedly = returncode == 0 and self.failure_expected
        failed_expectedly = returncode != 0 and self.failure_expected
        failed_unexpectedly = returncode != 0 and not self.failure_expected

        success = passed_expectedly or failed_expectedly

        # Print squish's output if needed:
        if s_verbose or returncode != 0:
            with s_stdoutLock:
                print(self.serverStdout)
                print(self.runnerStdout)

        if passed_unexpectedly:
            tag = '[XOK ]'
        elif passed_expectedly:
            tag = '[OK  ]'
        elif failed_expectedly:
            tag = '[XFAIL ]'
        elif failed_unexpectedly:
            tag = '[FAIL]'

        print('%s %s' % (tag, self.name))

        if success:
            self.numSuccesses += 1
        else:
            self.numFailures += 1

        return success


class Statistics:
    '''Simple struct just to contain the result of the test run'''

    def __init__(self, tests) -> None:
        self.__wasSuccessful = True
        self.__numTestsRan = 0
        self.__numTestsSkipped = 0
        self.__flakyTestNames = []
        self.__failedTestNames = []

        for test in tests:
            if test.wasSkipped():
                self.__numTestsSkipped += 1
            else:
                self.__numTestsRan += 1

                if test.wasFlaky():
                    self.__flakyTestNames.append(test.name)
                elif test.numFailures > 0:
                    self.__failedTestNames.append(test.name)

    def wasSucessful(self):
        '''Returns whether all tests were successful'''
        return len(self.__failedTestNames) == 0

    def printStats(self):
        print("Ran %d, skipped %d, failed %d, flaky %d" %
              (self.__numTestsRan, self.__numTestsSkipped, len(self.__failedTestNames), len(self.__flakyTestNames)))
        if self.__failedTestNames:
            print("Failed tests: %s" % ','.join(self.__failedTestNames))
        if self.__flakyTestNames:
            print("Flaky tests: %s" % ','.join(self.__flakyTestNames))


class SquishRunner:
    def __init__(self) -> None:
        self.tests = []
        self.aut = ''
        self.globalScriptDir = ''
        self.loadTests()

    def loadTests(self):
        contents = ''
        try:
            f = open(TESTS_JSON_FILENAME, 'r')
            contents = f.read()
            f.close()
        except Exception as e:
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

        if 'env' in decoded:
            for key in decoded['env']:
                os.environ[key] = decoded['env'][key]

        if 'outputFilters' in decoded:
            outputFilters = decoded['outputFilters']
            for filter in outputFilters:
                s_outputFilers.append(re.compile(filter))

        jsonTests = decoded['tests']
        for jsonTest in jsonTests:
            test = SquishTest.fromJson(jsonTest)
            test.globalScriptDir = self.globalScriptDir

            if not test.disabled:
                self.tests.append(test)

        if not self.aut:
            print("ERROR: aut attribute not found in %s" % TESTS_JSON_FILENAME)
            sys.exit(1)

    def chunks(self, num_chunks: int, requested_tests):
        '''Splits the list of requested tests into chunks. Returns a list of list of tests. At most num_chunks lists are returned'''
        list_of_chunks = [[] for x in range(num_chunks)]
        i = num_chunks
        for test in requested_tests:
            i = (i + 1) % num_chunks
            list_of_chunks[i].append(test)

        return list_of_chunks

    def run_in_parallel(self, num_jobs, requested_tests):
        '''Runs the requested tests, in parallel.'''

        self.createINIFiles()
        chunks = self.chunks(num_jobs, requested_tests)

        # Discard empty lists
        chunks = list(filter(lambda chunk: len(chunk) > 0, chunks))

        if s_verbose:
            print("Running a total of %d tests split through %d threads" %
                  (len(requested_tests), len(chunks)))

        threads = []
        for chunk in chunks:
            t = threading.Thread(target=self.run_in_sequence, args=(chunk,))
            t.start()
            threads.append(t)

        if s_verbose:
            print("Waiting for threads to finish...")

        for thread in threads:
            thread.join()

        if s_verbose:
            print("All threads finished.")

    def run_in_sequence(self, requested_tests):
        '''Runs the requested tests in sequence.'''

        if s_verbose:
            names = list(map(lambda test: test.name, requested_tests))
            print("Starting Thread to run: %d tests (%s)" %
                  (len(requested_tests), ",".join(names)))

        for test in requested_tests:
            test_success = self.run_single_test(test)

        if s_verbose:
            print("Finished thread for (%s)." % (",".join(names)))

    def run_single_test(self, test: SquishTest) -> bool:

        (accepted, skipReason) = self.acceptsTest(test)
        if not accepted:
            print('[SKIP] %s (%s)' % (test.name, skipReason))
            test.markSkipped()
            return True

        self.prepareEnv()
        while test.remainingRuns() > 0:  # if --maxFlakyRuns is passed, we'll execute until it passes
            success = test.run()
            if success:
                break

        # print("Finished running %s" % test.name)
        return success

    def createINIFiles(self):

        # Create ~/.squish/ver1/paths.ini - I don't see a way for it not to be global though.
        if self.globalScriptDir:
            if run_command_sync(['squishrunner', '--config', 'setGlobalScriptDirs', self.globalScriptDir]).returncode != 0:
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
        if run_command_sync(commonArgs + ['--config', 'setCursorAnimation', 'off']).returncode != 0:
            print("ERROR: Could not set cursor animation off")
            sys.exit(1)

        if run_command_sync(commonArgs + ['--config', 'addAUT', self.aut, s_autPath]).returncode != 0:
            print("ERROR: Could not set AUT %s/%s" % (self.aut, s_autPath))
            sys.exit(1)

    def printTests(self):
        for test in self.tests:
            print(test.name)

    def testByName(self, name):
        for test in self.tests:
            if test.name == name:
                return test
        return None

    def killChildProcesses(self):
        for test in self.tests:
            if test.runnerProc:
                kill_process(test.runnerProc)
            if test.serverProc:
                kill_process(test.serverProc)


class OffscreenSquishRunner(SquishRunner):
    def __init__(self):
        super().__init__()

    def prepareEnv(self):
        os.environ['QT_QPA_PLATFORM'] = 'offscreen'

    def acceptsTest(self, test: SquishTest):
        return (test.supportsOffscreen, 'Test does not support offscreen')


class NativeSquishRunner(SquishRunner):
    def __init__(self):
        super().__init__()

    def acceptsTest(self, test: SquishTest):
        return (True, '')

    def prepareEnv(self):
        os.environ['QT_QPA_PLATFORM'] = ''


parser = argparse.ArgumentParser(description=INSTRUCTIONS)
parser.add_argument('-v', '--verbose', help='Verbose mode',
                    action='store_true', required=False)
parser.add_argument('-l', '--list', help='Lists all tests',
                    action='store_true', required=False)
parser.add_argument(
    '-t', '--tests', help='Comma separated list of test names to run', required=False)
parser.add_argument('--native', help='Uses the native QPA instead of offscreen',
                    action='store_true', required=False)

parser.add_argument(
    '-o', '--outputdir', help='Directory to store the tests output.', required=False)

parser.add_argument(
    '-a', '--autPath', help='Directory containing the AUT. Required the first time only, since it will be written to server.ini', required=False)

parser.add_argument('squishdir', metavar='<Squish Directory>', nargs=1,
                    help='squish directory containing your tests and the %s file' % TESTS_JSON_FILENAME)

parser.add_argument(
    '-j', '--jobs', help='Number of squishrunner sessions to be run in parallel', required=False, type=int)

parser.add_argument(
    '--maxFlakyRuns', help='Runs a test at most N times until it passes.', required=False, type=int, default=1)

args = parser.parse_args()

s_verbose = args.verbose
if args.autPath:
    s_autPath = os.path.abspath(args.autPath)

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
plat = OffscreenSquishRunner() if s_isOffscreen else NativeSquishRunner()

if args.list:
    plat.printTests()
    sys.exit(0)

if args.outputdir:
    s_resultdir = args.outputdir
    if not os.path.isdir(args.outputdir):
        print("Did not find directory %s" % args.outputdir)
        sys.exit(1)

requestedTests = []
if args.tests:
    requestedTestNames = args.tests.split(",")
    for requestedTestName in requestedTestNames:
        test = plat.testByName(requestedTestName)
        if not test:
            print(
                f"Unknown test {requestedTestName}. Run with -l to see a list of tests.")
            sys.exit(-1)

        requestedTests.append(test)
else:
    requestedTests = plat.tests


if args.jobs:
    num_jobs = args.jobs
else:
    # When running on Native, we don't support paralellism
    num_jobs = s_num_cpus * 2 if s_isOffscreen else 1

try:
    plat.run_in_parallel(num_jobs, requestedTests)
except KeyboardInterrupt:
    print("Interrupted...")
    plat.killChildProcesses()
    sys.exit(1)

results = Statistics(requestedTests)
results.printStats()

print("Took %s seconds" % (int(time.time() - s_startTime)))

if results.wasSucessful():
    print("Success!")

plat.killChildProcesses()

sys.exit(0 if results.wasSucessful() else 1)
