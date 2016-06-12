
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/usr/local/nginx/sbin/nginx -t

	kill -USR2 `cat /ad_system/ad_system_58.com/conf/ad_system_display.pid`
	sleep 1
	test -f /ad_system/ad_system_58.com/conf/ad_system_display.pid.oldbin

	kill -QUIT `cat /ad_system/ad_system_58.com/conf/ad_system_display.pid.oldbin`
