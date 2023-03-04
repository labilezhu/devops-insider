

> https://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/ehframechpt.html#EHFRAME
> https://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-PDA/LSB-PDA/ehframechpt.html

Chapter 8. Exception Frames
When using languages that support exceptions, such as C++, additional information must be provided to the runtime environment that describes the call frames that much be unwound during the processing of an exception. This information is contained in the special sections .eh_frame and .eh_framehdr.

Note: The format of the .eh_frame section is similar in format and purpose to the .debug_frame section which is specified in DWARF Debugging Information Format, Revision 3.0.0 (Draft). Readers are advised that there are some subtle difference, and care should be taken when comparing the two sections.

> http://dandylife.net/blog/archives/686

