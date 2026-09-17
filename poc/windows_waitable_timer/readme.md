# Windows Waitable Timer

This is a small POC to demonstrate how Windows Waitable Timer objects work.

They make use of the kernel to wait, instead of the less precise Sleep method.
This also frees up CPU in the thread waiting as the kernel can schedule other
things during the wait.

In the example, a time period of 100 ms is used. The documentation suggests a longer
time such as seconds because of mobile CPU power consumption.
