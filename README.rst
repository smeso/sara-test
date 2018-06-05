=========
sara-test
=========

sara-test is a test suite for S.A.R.A. LSM.

To install it run:
	make
	
	sudo make install

To uninstall it run:
	sudo make uninstall

To use it without installing it, put the following
lines in your sara wxprot config file::

	SOURCE_PATH/bin/* mprotect,verbose
	SOURCE_PATH/bin/procattr mmap,other,complain,verbose
	SOURCE_PATH/bin/fake_tramp mprotect,emutramp_or_mprotect,verbose
	SOURCE_PATH/bin/trampoline* mprotect,emutramp_or_mprotect,verbose

and then run:
	EXTRA_BINS_PATH="." make && cd bin && ./sara-test

Please note that, due to the need to create GCC trampolines, sara-test must
be compiled using GCC.

You can find more information on S.A.R.A. at <https://smeso.it/sara>
