# Linux Kernel module with faults

This is a toy Linux Kernel module, with intentional (and perhaps
unintentional) coding faults. It is used as an illustration for errors
caused by different types of programming errors.

## Usage

Make sure you have installed kernel headers or sources, for example in
Ubuntu with
```
$ sudo apt show linux-headers-`uname -r`
```

Then compile the module with

    $ make

Load the compiled module with

    $ sudo insmod faulty.ko
	
and if you look at the kernel messages with `dmesg -k` you should see
a message indicating that the module is loaded. If you get an error
message saying something about _kernel lockdown_, you can either
disable Secure Boot in BIOS (easy) or sign the compiled module (not so
easy).

You can unload the module with

    $ sudo rmmod faulty

*TODO*: examples of faults

## Author

Ilja Sidoroff (-at) iki.fi

## License

GNU GPLv2
