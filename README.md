Apple // Bejeweled
========

This is an implementation of Bejeweled written for the Apple //.  It is my HackFest entry at KansasFest 2016.

![A2Bejwld Screenshot](/a2bejwld.png "Apple // Bejeweled Screenshot")

[Download a disk image](https://github.com/jeremysrand/a2bejwld/releases/download/2.7/a2bejwld.dsk)

[Watch the YouTube video](https://youtu.be/yseAGBzREik)

Bugs:
=====
    * When built using cc65 v2.19, the mockingboard sound seems to stop working correctly part way through a game
      This is testing using Virtual II.  The build using cc65 v2.17 seemed to be fine.
    * Also, if I disable/reenable mockingboard sounds in options, the computer crashes with RESTART SYSTEM-$01
      This is almost definitely an interrupt handler problem.
