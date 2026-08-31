# Compile Time String Replacement

This is a simple example of compile time string replacement in a const char*.

The replacement happens in consteval functions which are evaluated at compile time.

The result of the replacement (the const char*) is stored in the executables data section.

The project can be the start of a fictive printf() or other formatting function which optimizes away the 's' in "%s"
and replaces it with a character not used in logging '\01' or also written as 0x01.

# LLM Use

Please note that it has been written with LLM assistance / review
which does not indemnify the user of the code.
