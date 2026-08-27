# Return Value Optimized optional 

It is possible to use std::optional in a way that it incurs
extra move and destructor calls.

This demonstrates how to enforce the "right way" which is also possible
with std::optional but which is a bit cubmersome.

This class does away with the convenience move assignment operator.

# LLM Use

Please note that it has been written with LLM assistance / review
which does not indemnify the user of the code.
