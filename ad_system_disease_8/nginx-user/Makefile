
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

	kill -USR2 `cat /ad_system/ad_system_disease_8/conf/ad_system_user.pid`
	sleep 1
	test -f /ad_system/ad_system_disease_8/conf/ad_system_user.pid.oldbin

	kill -QUIT `cat /ad_system/ad_system_disease_8/conf/ad_system_user.pid.oldbin`
