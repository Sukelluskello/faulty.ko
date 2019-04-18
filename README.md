# Linux Kernel module with faults

This is a toy Linux Kernel module, with intentional (and perhaps
unintentional) coding faults. It is used as an illustration and test
case for errors caused by different types of programming errors.

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

You can unload the module with (if playing with bugs hasn't made your
kernel unstable/locked)

    $ sudo rmmod faulty

The module exposes endpoints in `debugfs` (usually mounted in
`/sys/kernel/debugfs`), which can be used to trigger faults. This
might crash or lock your kernel, so using a virtual machine might be a
good idea, a Vagrantfile is provided.

You can usually read from or write to these endpoints using e.g.

    # cat /sys/kernel/debugs/faulty/sbo
	# echo -n "123" > /sys/kernel/debugfs/faulty/sbo
	
(accessing `debugfs` is easiest as root) 

Exposed endpoints:

  * `data-race` (r/w): a write will write the same thing into two buffers, read will return the contents of the first buffer, kernel will notify, is the buffer contents aren't the same
  * `double-free` (r/w): reading will allocate a buffer, writing will free it, double-free error can be triggered with two subsequent writes
  * `format` (r/w): write will get passed directly to printk-function
  * `infoleak` (r): reading will return uninitialized memory
  * `overflow` (r): reading will increment unsigned counter, which will overflow
  * `sbo` (r/w): a stack buffer overflow, write more than 10 bytes to trigger
  * `slob` (r/w): a buffer overflow in slab area, write more than 10 bytes to trigger
  * `underflow` (r): reading will decrement signed counter, which will underflow
  * `use-after-free`  (r): reading will allocate and free memory and then try to access it

## Author

Ilja Sidoroff (-at) iki.fi

## License

GNU GPLv2
