Future
------

* Modernize code
* Integrate with systemd/logind.
* Address i18n issues on the bugtracker.

**__Issues below here are 7 years old, kept for historical purposes.__**

1.2.2
-----

* drawing problem on screens <= 1024x768 (implemented)
* Don't start X server if theme's not found

1.2.x
-----
* i18n
* don't choose a non-existing theme in random selection
* think about multi-line configuration:
    current_theme = t1, t2, t3
    current_theme = t4, t5
  or
    current_theme = t1, t2, t3 \
		    t4, t5
* FreeBSD fixes

2.0.x
-----
* restart X server on ctrl-alt-backspace
* allow switching theme on the fly if a set is specified in slim.conf
* text alignment, to allow to center texts of unknown length (e.g. %host)
