# /include/uapi

> https://stackoverflow.com/questions/18858190/whats-in-include-uapi-of-kernel-source-project



The `uapi` folder is supposed to contain the user space API of the kernel. Then upon kernel installation, the uapi include files become the top level /usr/include/linux/ files. (I'm not entirely clear on what exceptions remain.)

The other headers in theory are then private to the kernel. This allow clean separation of the user-visible and kernel-only structures which previously were intermingled in a single header file.

The best discussion I have seen of this is [located at a Linux Weekly News](http://lwn.net/Articles/507794/) article that predates the patch landing.

The UAPI patch itself landed with kernel 3.7. Linus's [quick and dirty summary](http://lwn.net/Articles/519762/) is:

> - the "uapi" include file cleanups. The idea is that the stuff exported to user space should now be found under include/uapi and arch/$(ARCH)/include/uapi.
>
>   Let's hope it actually works. Because otherwise this was just a totally pointless pain in the *ss. And regardless, I'm definitely done with these kinds of "let's do massive cleanup of the include files" forever.
