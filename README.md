dell_service_tag - DELL Service Tag CMOS read/change utility
============================================================
This utility reads/writes service tag from/to CMOS image on DELL machines.

Useful when cloning machines including CMOS settings - to preserve service
tags (which would be cloned too otherwise).

Requirements:
-------------
Linux, GCC, Make

Compilation:
------------
    $ make

Usage:
------
Can be used directly on /dev/nvram or on an image created e.g. by:

    # dd if=/dev/nvram of=cmos.bin

If you don't have /dev/nvram, try:

    # modprobe nvram

Example:
--------
    # dell_service_tag /dev/nvram
    A123456
    # dell_service_tag /dev/nvram B234567
    # dell_service_tag /dev/nvram
    B234567

Cloning:
--------
    # cat cmos.bin >/dev/nvram
    # dell_service_tag /dev/nvram `dmidecode -s system-serial-number`

Tested and working machines:
----------------------------
OptiPlex 745, OptiPlex 755, OptiPlex 760, OptiPlex 780
