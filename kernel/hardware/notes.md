# Recording a very tricky bug
 
I use '#progma pack(1)' to pack some special structure in filesystems
BUT , I forget to use '#progma pack()' to cancel the custom pack instruction
SO , When one file include this header ^^^^ , and the other file doesn's, and they operate one same structure, the offset calculated by compiler will be different, so, something bad happened!
