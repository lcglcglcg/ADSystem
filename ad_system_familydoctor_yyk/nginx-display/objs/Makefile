
CC =	cc
CFLAGS =  -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g 
CPP =	cc -E
LINK =	$(CC)


ALL_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src/lg_core \
	-I src/lg_json \
	-I src/lg_ocilib/include \
	-I src/lg_ocilib/include/oci \
	-I src/lg_ocilib/include/ocilib \
	-I src/ad_system_display_module/include \
	-I objs \
	-I src/http \
	-I src/http/modules \
	-I src/mail


CORE_DEPS = src/core/nginx.h \
	src/core/ngx_config.h \
	src/core/ngx_core.h \
	src/core/ngx_log.h \
	src/core/ngx_palloc.h \
	src/core/ngx_array.h \
	src/core/ngx_list.h \
	src/core/ngx_hash.h \
	src/core/ngx_buf.h \
	src/core/ngx_queue.h \
	src/core/ngx_string.h \
	src/core/ngx_parse.h \
	src/core/ngx_inet.h \
	src/core/ngx_file.h \
	src/core/ngx_crc.h \
	src/core/ngx_crc32.h \
	src/core/ngx_murmurhash.h \
	src/core/ngx_md5.h \
	src/core/ngx_sha1.h \
	src/core/ngx_rbtree.h \
	src/core/ngx_radix_tree.h \
	src/core/ngx_slab.h \
	src/core/ngx_times.h \
	src/core/ngx_shmtx.h \
	src/core/ngx_connection.h \
	src/core/ngx_cycle.h \
	src/core/ngx_conf_file.h \
	src/core/ngx_resolver.h \
	src/core/ngx_open_file_cache.h \
	src/core/ngx_crypt.h \
	src/core/ngx_proxy_protocol.h \
	src/core/ngx_syslog.h \
	src/event/ngx_event.h \
	src/event/ngx_event_timer.h \
	src/event/ngx_event_posted.h \
	src/event/ngx_event_connect.h \
	src/event/ngx_event_pipe.h \
	src/os/unix/ngx_time.h \
	src/os/unix/ngx_errno.h \
	src/os/unix/ngx_alloc.h \
	src/os/unix/ngx_files.h \
	src/os/unix/ngx_channel.h \
	src/os/unix/ngx_shmem.h \
	src/os/unix/ngx_process.h \
	src/os/unix/ngx_setaffinity.h \
	src/os/unix/ngx_setproctitle.h \
	src/os/unix/ngx_atomic.h \
	src/os/unix/ngx_gcc_atomic_x86.h \
	src/os/unix/ngx_thread.h \
	src/os/unix/ngx_socket.h \
	src/os/unix/ngx_os.h \
	src/os/unix/ngx_user.h \
	src/os/unix/ngx_process_cycle.h \
	src/os/unix/ngx_linux_config.h \
	src/os/unix/ngx_linux.h \
	src/core/ngx_regex.h \
	objs/ngx_auto_config.h


CORE_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src/lg_core \
	-I src/lg_json \
	-I src/lg_ocilib/include \
	-I src/lg_ocilib/include/oci \
	-I src/lg_ocilib/include/ocilib \
	-I src/ad_system_display_module/include \
	-I objs


HTTP_DEPS = src/http/ngx_http.h \
	src/http/ngx_http_request.h \
	src/http/ngx_http_config.h \
	src/http/ngx_http_core_module.h \
	src/http/ngx_http_cache.h \
	src/http/ngx_http_variables.h \
	src/http/ngx_http_script.h \
	src/http/ngx_http_upstream.h \
	src/http/ngx_http_upstream_round_robin.h \
	src/http/modules/ngx_http_ssi_filter_module.h


HTTP_INCS = -I src/http \
	-I src/http/modules


ADDON_DEPS = $(CORE_DEPS)  $(HTTP_DEPS)


objs/nginx:	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/src/core/ngx_palloc.o \
	objs/src/core/ngx_array.o \
	objs/src/core/ngx_list.o \
	objs/src/core/ngx_hash.o \
	objs/src/core/ngx_buf.o \
	objs/src/core/ngx_queue.o \
	objs/src/core/ngx_output_chain.o \
	objs/src/core/ngx_string.o \
	objs/src/core/ngx_parse.o \
	objs/src/core/ngx_inet.o \
	objs/src/core/ngx_file.o \
	objs/src/core/ngx_crc32.o \
	objs/src/core/ngx_murmurhash.o \
	objs/src/core/ngx_md5.o \
	objs/src/core/ngx_rbtree.o \
	objs/src/core/ngx_radix_tree.o \
	objs/src/core/ngx_slab.o \
	objs/src/core/ngx_times.o \
	objs/src/core/ngx_shmtx.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_spinlock.o \
	objs/src/core/ngx_cpuinfo.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/core/ngx_resolver.o \
	objs/src/core/ngx_open_file_cache.o \
	objs/src/core/ngx_crypt.o \
	objs/src/core/ngx_proxy_protocol.o \
	objs/src/core/ngx_syslog.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/event/ngx_event_connect.o \
	objs/src/event/ngx_event_pipe.o \
	objs/src/os/unix/ngx_time.o \
	objs/src/os/unix/ngx_errno.o \
	objs/src/os/unix/ngx_alloc.o \
	objs/src/os/unix/ngx_files.o \
	objs/src/os/unix/ngx_socket.o \
	objs/src/os/unix/ngx_recv.o \
	objs/src/os/unix/ngx_readv_chain.o \
	objs/src/os/unix/ngx_udp_recv.o \
	objs/src/os/unix/ngx_send.o \
	objs/src/os/unix/ngx_writev_chain.o \
	objs/src/os/unix/ngx_channel.o \
	objs/src/os/unix/ngx_shmem.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_setproctitle.o \
	objs/src/os/unix/ngx_posix_init.o \
	objs/src/os/unix/ngx_user.o \
	objs/src/os/unix/ngx_process_cycle.o \
	objs/src/os/unix/ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/os/unix/ngx_linux_sendfile_chain.o \
	objs/src/core/ngx_regex.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/ngx_http_special_response.o \
	objs/src/http/ngx_http_request.o \
	objs/src/http/ngx_http_parse.o \
	objs/src/http/ngx_http_header_filter_module.o \
	objs/src/http/ngx_http_write_filter_module.o \
	objs/src/http/ngx_http_copy_filter_module.o \
	objs/src/http/modules/ngx_http_log_module.o \
	objs/src/http/ngx_http_request_body.o \
	objs/src/http/ngx_http_variables.o \
	objs/src/http/ngx_http_script.o \
	objs/src/http/ngx_http_upstream.o \
	objs/src/http/ngx_http_upstream_round_robin.o \
	objs/src/http/ngx_http_parse_time.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/http/modules/ngx_http_index_module.o \
	objs/src/http/modules/ngx_http_chunked_filter_module.o \
	objs/src/http/modules/ngx_http_range_filter_module.o \
	objs/src/http/modules/ngx_http_headers_filter_module.o \
	objs/src/http/modules/ngx_http_not_modified_filter_module.o \
	objs/src/http/ngx_http_file_cache.o \
	objs/src/http/modules/ngx_http_gzip_filter_module.o \
	objs/src/http/ngx_http_postpone_filter_module.o \
	objs/src/http/modules/ngx_http_ssi_filter_module.o \
	objs/src/http/modules/ngx_http_charset_filter_module.o \
	objs/src/http/modules/ngx_http_userid_filter_module.o \
	objs/src/http/modules/ngx_http_autoindex_module.o \
	objs/src/http/modules/ngx_http_auth_basic_module.o \
	objs/src/http/modules/ngx_http_access_module.o \
	objs/src/http/modules/ngx_http_limit_conn_module.o \
	objs/src/http/modules/ngx_http_limit_req_module.o \
	objs/src/http/modules/ngx_http_geo_module.o \
	objs/src/http/modules/ngx_http_map_module.o \
	objs/src/http/modules/ngx_http_split_clients_module.o \
	objs/src/http/modules/ngx_http_referer_module.o \
	objs/src/http/modules/ngx_http_rewrite_module.o \
	objs/src/http/modules/ngx_http_proxy_module.o \
	objs/src/http/modules/ngx_http_fastcgi_module.o \
	objs/src/http/modules/ngx_http_uwsgi_module.o \
	objs/src/http/modules/ngx_http_scgi_module.o \
	objs/src/http/modules/ngx_http_memcached_module.o \
	objs/src/http/modules/ngx_http_empty_gif_module.o \
	objs/src/http/modules/ngx_http_browser_module.o \
	objs/src/http/modules/ngx_http_upstream_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
	objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
	objs/addon/lg_core/lg_config_file.o \
	objs/addon/lg_core/lg_db.o \
	objs/addon/lg_core/lg_list.o \
	objs/addon/lg_core/lg_ngx_network.o \
	objs/addon/lg_core/lg_ngx_session.o \
	objs/addon/lg_core/lg_ngx_upstream.o \
	objs/addon/lg_core/lg_ngx_uri.o \
	objs/addon/lg_core/lg_ocilib.o \
	objs/addon/lg_core/lg_rbtree.o \
	objs/addon/lg_core/lg_string.o \
	objs/addon/lg_json/json_array.o \
	objs/addon/lg_json/json_hash.o \
	objs/addon/lg_json/json_object.o \
	objs/addon/lg_json/json_print.o \
	objs/addon/lg_json/json_tokener.o \
	objs/addon/lg_json/lg_json.o \
	objs/addon/lg_ocilib/ocilib_agent.o \
	objs/addon/lg_ocilib/ocilib_array.o \
	objs/addon/lg_ocilib/ocilib_bind.o \
	objs/addon/lg_ocilib/ocilib_callback.o \
	objs/addon/lg_ocilib/ocilib_collection.o \
	objs/addon/lg_ocilib/ocilib_column.o \
	objs/addon/lg_ocilib/ocilib_connection.o \
	objs/addon/lg_ocilib/ocilib_date.o \
	objs/addon/lg_ocilib/ocilib_define.o \
	objs/addon/lg_ocilib/ocilib_dequeue.o \
	objs/addon/lg_ocilib/ocilib_dirpath.o \
	objs/addon/lg_ocilib/ocilib_element.o \
	objs/addon/lg_ocilib/ocilib_enqueue.o \
	objs/addon/lg_ocilib/ocilib_error.o \
	objs/addon/lg_ocilib/ocilib_event.o \
	objs/addon/lg_ocilib/ocilib_exception.o \
	objs/addon/lg_ocilib/ocilib_file.o \
	objs/addon/lg_ocilib/ocilib_format.o \
	objs/addon/lg_ocilib/ocilib_handle.o \
	objs/addon/lg_ocilib/ocilib_hash.o \
	objs/addon/lg_ocilib/ocilib_interval.o \
	objs/addon/lg_ocilib/ocilib_iterator.o \
	objs/addon/lg_ocilib/ocilib_library.o \
	objs/addon/lg_ocilib/ocilib_list.o \
	objs/addon/lg_ocilib/ocilib_lob.o \
	objs/addon/lg_ocilib/ocilib_long.o \
	objs/addon/lg_ocilib/ocilib_memory.o \
	objs/addon/lg_ocilib/ocilib_msg.o \
	objs/addon/lg_ocilib/ocilib_mutex.o \
	objs/addon/lg_ocilib/ocilib_number.o \
	objs/addon/lg_ocilib/ocilib_object.o \
	objs/addon/lg_ocilib/ocilib_pool.o \
	objs/addon/lg_ocilib/ocilib_queue.o \
	objs/addon/lg_ocilib/ocilib_ref.o \
	objs/addon/lg_ocilib/ocilib_resultset.o \
	objs/addon/lg_ocilib/ocilib_statement.o \
	objs/addon/lg_ocilib/ocilib_string.o \
	objs/addon/lg_ocilib/ocilib_subscription.o \
	objs/addon/lg_ocilib/ocilib_thread.o \
	objs/addon/lg_ocilib/ocilib_threadkey.o \
	objs/addon/lg_ocilib/ocilib_timestamp.o \
	objs/addon/lg_ocilib/ocilib_transaction.o \
	objs/addon/lg_ocilib/ocilib_typeinfo.o \
	objs/addon/ad_system_display_module/ad_system_db_element.o \
	objs/addon/ad_system_display_module/ad_system_db_load.o \
	objs/addon/ad_system_display_module/ad_system_db_module.o \
	objs/addon/ad_system_display_module/ad_system_db_packet.o \
	objs/addon/ad_system_display_module/ad_system_db_region_code.o \
	objs/addon/ad_system_display_module/ad_system_db_select.o \
	objs/addon/ad_system_display_module/lg_ip_addr_find.o \
	objs/ngx_modules.o

	$(LINK) -o objs/nginx \
	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/src/core/ngx_palloc.o \
	objs/src/core/ngx_array.o \
	objs/src/core/ngx_list.o \
	objs/src/core/ngx_hash.o \
	objs/src/core/ngx_buf.o \
	objs/src/core/ngx_queue.o \
	objs/src/core/ngx_output_chain.o \
	objs/src/core/ngx_string.o \
	objs/src/core/ngx_parse.o \
	objs/src/core/ngx_inet.o \
	objs/src/core/ngx_file.o \
	objs/src/core/ngx_crc32.o \
	objs/src/core/ngx_murmurhash.o \
	objs/src/core/ngx_md5.o \
	objs/src/core/ngx_rbtree.o \
	objs/src/core/ngx_radix_tree.o \
	objs/src/core/ngx_slab.o \
	objs/src/core/ngx_times.o \
	objs/src/core/ngx_shmtx.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_spinlock.o \
	objs/src/core/ngx_cpuinfo.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/core/ngx_resolver.o \
	objs/src/core/ngx_open_file_cache.o \
	objs/src/core/ngx_crypt.o \
	objs/src/core/ngx_proxy_protocol.o \
	objs/src/core/ngx_syslog.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/event/ngx_event_connect.o \
	objs/src/event/ngx_event_pipe.o \
	objs/src/os/unix/ngx_time.o \
	objs/src/os/unix/ngx_errno.o \
	objs/src/os/unix/ngx_alloc.o \
	objs/src/os/unix/ngx_files.o \
	objs/src/os/unix/ngx_socket.o \
	objs/src/os/unix/ngx_recv.o \
	objs/src/os/unix/ngx_readv_chain.o \
	objs/src/os/unix/ngx_udp_recv.o \
	objs/src/os/unix/ngx_send.o \
	objs/src/os/unix/ngx_writev_chain.o \
	objs/src/os/unix/ngx_channel.o \
	objs/src/os/unix/ngx_shmem.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_setproctitle.o \
	objs/src/os/unix/ngx_posix_init.o \
	objs/src/os/unix/ngx_user.o \
	objs/src/os/unix/ngx_process_cycle.o \
	objs/src/os/unix/ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/os/unix/ngx_linux_sendfile_chain.o \
	objs/src/core/ngx_regex.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/ngx_http_special_response.o \
	objs/src/http/ngx_http_request.o \
	objs/src/http/ngx_http_parse.o \
	objs/src/http/ngx_http_header_filter_module.o \
	objs/src/http/ngx_http_write_filter_module.o \
	objs/src/http/ngx_http_copy_filter_module.o \
	objs/src/http/modules/ngx_http_log_module.o \
	objs/src/http/ngx_http_request_body.o \
	objs/src/http/ngx_http_variables.o \
	objs/src/http/ngx_http_script.o \
	objs/src/http/ngx_http_upstream.o \
	objs/src/http/ngx_http_upstream_round_robin.o \
	objs/src/http/ngx_http_parse_time.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/http/modules/ngx_http_index_module.o \
	objs/src/http/modules/ngx_http_chunked_filter_module.o \
	objs/src/http/modules/ngx_http_range_filter_module.o \
	objs/src/http/modules/ngx_http_headers_filter_module.o \
	objs/src/http/modules/ngx_http_not_modified_filter_module.o \
	objs/src/http/ngx_http_file_cache.o \
	objs/src/http/modules/ngx_http_gzip_filter_module.o \
	objs/src/http/ngx_http_postpone_filter_module.o \
	objs/src/http/modules/ngx_http_ssi_filter_module.o \
	objs/src/http/modules/ngx_http_charset_filter_module.o \
	objs/src/http/modules/ngx_http_userid_filter_module.o \
	objs/src/http/modules/ngx_http_autoindex_module.o \
	objs/src/http/modules/ngx_http_auth_basic_module.o \
	objs/src/http/modules/ngx_http_access_module.o \
	objs/src/http/modules/ngx_http_limit_conn_module.o \
	objs/src/http/modules/ngx_http_limit_req_module.o \
	objs/src/http/modules/ngx_http_geo_module.o \
	objs/src/http/modules/ngx_http_map_module.o \
	objs/src/http/modules/ngx_http_split_clients_module.o \
	objs/src/http/modules/ngx_http_referer_module.o \
	objs/src/http/modules/ngx_http_rewrite_module.o \
	objs/src/http/modules/ngx_http_proxy_module.o \
	objs/src/http/modules/ngx_http_fastcgi_module.o \
	objs/src/http/modules/ngx_http_uwsgi_module.o \
	objs/src/http/modules/ngx_http_scgi_module.o \
	objs/src/http/modules/ngx_http_memcached_module.o \
	objs/src/http/modules/ngx_http_empty_gif_module.o \
	objs/src/http/modules/ngx_http_browser_module.o \
	objs/src/http/modules/ngx_http_upstream_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
	objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
	objs/addon/lg_core/lg_config_file.o \
	objs/addon/lg_core/lg_db.o \
	objs/addon/lg_core/lg_list.o \
	objs/addon/lg_core/lg_ngx_network.o \
	objs/addon/lg_core/lg_ngx_session.o \
	objs/addon/lg_core/lg_ngx_upstream.o \
	objs/addon/lg_core/lg_ngx_uri.o \
	objs/addon/lg_core/lg_ocilib.o \
	objs/addon/lg_core/lg_rbtree.o \
	objs/addon/lg_core/lg_string.o \
	objs/addon/lg_json/json_array.o \
	objs/addon/lg_json/json_hash.o \
	objs/addon/lg_json/json_object.o \
	objs/addon/lg_json/json_print.o \
	objs/addon/lg_json/json_tokener.o \
	objs/addon/lg_json/lg_json.o \
	objs/addon/lg_ocilib/ocilib_agent.o \
	objs/addon/lg_ocilib/ocilib_array.o \
	objs/addon/lg_ocilib/ocilib_bind.o \
	objs/addon/lg_ocilib/ocilib_callback.o \
	objs/addon/lg_ocilib/ocilib_collection.o \
	objs/addon/lg_ocilib/ocilib_column.o \
	objs/addon/lg_ocilib/ocilib_connection.o \
	objs/addon/lg_ocilib/ocilib_date.o \
	objs/addon/lg_ocilib/ocilib_define.o \
	objs/addon/lg_ocilib/ocilib_dequeue.o \
	objs/addon/lg_ocilib/ocilib_dirpath.o \
	objs/addon/lg_ocilib/ocilib_element.o \
	objs/addon/lg_ocilib/ocilib_enqueue.o \
	objs/addon/lg_ocilib/ocilib_error.o \
	objs/addon/lg_ocilib/ocilib_event.o \
	objs/addon/lg_ocilib/ocilib_exception.o \
	objs/addon/lg_ocilib/ocilib_file.o \
	objs/addon/lg_ocilib/ocilib_format.o \
	objs/addon/lg_ocilib/ocilib_handle.o \
	objs/addon/lg_ocilib/ocilib_hash.o \
	objs/addon/lg_ocilib/ocilib_interval.o \
	objs/addon/lg_ocilib/ocilib_iterator.o \
	objs/addon/lg_ocilib/ocilib_library.o \
	objs/addon/lg_ocilib/ocilib_list.o \
	objs/addon/lg_ocilib/ocilib_lob.o \
	objs/addon/lg_ocilib/ocilib_long.o \
	objs/addon/lg_ocilib/ocilib_memory.o \
	objs/addon/lg_ocilib/ocilib_msg.o \
	objs/addon/lg_ocilib/ocilib_mutex.o \
	objs/addon/lg_ocilib/ocilib_number.o \
	objs/addon/lg_ocilib/ocilib_object.o \
	objs/addon/lg_ocilib/ocilib_pool.o \
	objs/addon/lg_ocilib/ocilib_queue.o \
	objs/addon/lg_ocilib/ocilib_ref.o \
	objs/addon/lg_ocilib/ocilib_resultset.o \
	objs/addon/lg_ocilib/ocilib_statement.o \
	objs/addon/lg_ocilib/ocilib_string.o \
	objs/addon/lg_ocilib/ocilib_subscription.o \
	objs/addon/lg_ocilib/ocilib_thread.o \
	objs/addon/lg_ocilib/ocilib_threadkey.o \
	objs/addon/lg_ocilib/ocilib_timestamp.o \
	objs/addon/lg_ocilib/ocilib_transaction.o \
	objs/addon/lg_ocilib/ocilib_typeinfo.o \
	objs/addon/ad_system_display_module/ad_system_db_element.o \
	objs/addon/ad_system_display_module/ad_system_db_load.o \
	objs/addon/ad_system_display_module/ad_system_db_module.o \
	objs/addon/ad_system_display_module/ad_system_db_packet.o \
	objs/addon/ad_system_display_module/ad_system_db_region_code.o \
	objs/addon/ad_system_display_module/ad_system_db_select.o \
	objs/addon/ad_system_display_module/lg_ip_addr_find.o \
	objs/ngx_modules.o \
	-lpthread -lcrypt -luuid -L/ad_system/ad_system_lib/oci_lib/ -lnnz -lclntsh -lpcre -lcrypto -lcrypto -lz
	


objs/ngx_modules.o:	$(CORE_DEPS) \
	objs/ngx_modules.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/ngx_modules.o \
		objs/ngx_modules.c


objs/src/core/nginx.o:	$(CORE_DEPS) \
	src/core/nginx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/nginx.o \
		src/core/nginx.c


objs/src/core/ngx_log.o:	$(CORE_DEPS) \
	src/core/ngx_log.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_log.o \
		src/core/ngx_log.c


objs/src/core/ngx_palloc.o:	$(CORE_DEPS) \
	src/core/ngx_palloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_palloc.o \
		src/core/ngx_palloc.c


objs/src/core/ngx_array.o:	$(CORE_DEPS) \
	src/core/ngx_array.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_array.o \
		src/core/ngx_array.c


objs/src/core/ngx_list.o:	$(CORE_DEPS) \
	src/core/ngx_list.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_list.o \
		src/core/ngx_list.c


objs/src/core/ngx_hash.o:	$(CORE_DEPS) \
	src/core/ngx_hash.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_hash.o \
		src/core/ngx_hash.c


objs/src/core/ngx_buf.o:	$(CORE_DEPS) \
	src/core/ngx_buf.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_buf.o \
		src/core/ngx_buf.c


objs/src/core/ngx_queue.o:	$(CORE_DEPS) \
	src/core/ngx_queue.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_queue.o \
		src/core/ngx_queue.c


objs/src/core/ngx_output_chain.o:	$(CORE_DEPS) \
	src/core/ngx_output_chain.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_output_chain.o \
		src/core/ngx_output_chain.c


objs/src/core/ngx_string.o:	$(CORE_DEPS) \
	src/core/ngx_string.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_string.o \
		src/core/ngx_string.c


objs/src/core/ngx_parse.o:	$(CORE_DEPS) \
	src/core/ngx_parse.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_parse.o \
		src/core/ngx_parse.c


objs/src/core/ngx_inet.o:	$(CORE_DEPS) \
	src/core/ngx_inet.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_inet.o \
		src/core/ngx_inet.c


objs/src/core/ngx_file.o:	$(CORE_DEPS) \
	src/core/ngx_file.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_file.o \
		src/core/ngx_file.c


objs/src/core/ngx_crc32.o:	$(CORE_DEPS) \
	src/core/ngx_crc32.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_crc32.o \
		src/core/ngx_crc32.c


objs/src/core/ngx_murmurhash.o:	$(CORE_DEPS) \
	src/core/ngx_murmurhash.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_murmurhash.o \
		src/core/ngx_murmurhash.c


objs/src/core/ngx_md5.o:	$(CORE_DEPS) \
	src/core/ngx_md5.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_md5.o \
		src/core/ngx_md5.c


objs/src/core/ngx_rbtree.o:	$(CORE_DEPS) \
	src/core/ngx_rbtree.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_rbtree.o \
		src/core/ngx_rbtree.c


objs/src/core/ngx_radix_tree.o:	$(CORE_DEPS) \
	src/core/ngx_radix_tree.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_radix_tree.o \
		src/core/ngx_radix_tree.c


objs/src/core/ngx_slab.o:	$(CORE_DEPS) \
	src/core/ngx_slab.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_slab.o \
		src/core/ngx_slab.c


objs/src/core/ngx_times.o:	$(CORE_DEPS) \
	src/core/ngx_times.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_times.o \
		src/core/ngx_times.c


objs/src/core/ngx_shmtx.o:	$(CORE_DEPS) \
	src/core/ngx_shmtx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_shmtx.o \
		src/core/ngx_shmtx.c


objs/src/core/ngx_connection.o:	$(CORE_DEPS) \
	src/core/ngx_connection.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_connection.o \
		src/core/ngx_connection.c


objs/src/core/ngx_cycle.o:	$(CORE_DEPS) \
	src/core/ngx_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_cycle.o \
		src/core/ngx_cycle.c


objs/src/core/ngx_spinlock.o:	$(CORE_DEPS) \
	src/core/ngx_spinlock.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_spinlock.o \
		src/core/ngx_spinlock.c


objs/src/core/ngx_cpuinfo.o:	$(CORE_DEPS) \
	src/core/ngx_cpuinfo.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_cpuinfo.o \
		src/core/ngx_cpuinfo.c


objs/src/core/ngx_conf_file.o:	$(CORE_DEPS) \
	src/core/ngx_conf_file.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_conf_file.o \
		src/core/ngx_conf_file.c


objs/src/core/ngx_resolver.o:	$(CORE_DEPS) \
	src/core/ngx_resolver.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_resolver.o \
		src/core/ngx_resolver.c


objs/src/core/ngx_open_file_cache.o:	$(CORE_DEPS) \
	src/core/ngx_open_file_cache.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_open_file_cache.o \
		src/core/ngx_open_file_cache.c


objs/src/core/ngx_crypt.o:	$(CORE_DEPS) \
	src/core/ngx_crypt.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_crypt.o \
		src/core/ngx_crypt.c


objs/src/core/ngx_proxy_protocol.o:	$(CORE_DEPS) \
	src/core/ngx_proxy_protocol.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_proxy_protocol.o \
		src/core/ngx_proxy_protocol.c


objs/src/core/ngx_syslog.o:	$(CORE_DEPS) \
	src/core/ngx_syslog.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_syslog.o \
		src/core/ngx_syslog.c


objs/src/event/ngx_event.o:	$(CORE_DEPS) \
	src/event/ngx_event.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event.o \
		src/event/ngx_event.c


objs/src/event/ngx_event_timer.o:	$(CORE_DEPS) \
	src/event/ngx_event_timer.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_timer.o \
		src/event/ngx_event_timer.c


objs/src/event/ngx_event_posted.o:	$(CORE_DEPS) \
	src/event/ngx_event_posted.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_posted.o \
		src/event/ngx_event_posted.c


objs/src/event/ngx_event_accept.o:	$(CORE_DEPS) \
	src/event/ngx_event_accept.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_accept.o \
		src/event/ngx_event_accept.c


objs/src/event/ngx_event_connect.o:	$(CORE_DEPS) \
	src/event/ngx_event_connect.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_connect.o \
		src/event/ngx_event_connect.c


objs/src/event/ngx_event_pipe.o:	$(CORE_DEPS) \
	src/event/ngx_event_pipe.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_pipe.o \
		src/event/ngx_event_pipe.c


objs/src/os/unix/ngx_time.o:	$(CORE_DEPS) \
	src/os/unix/ngx_time.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_time.o \
		src/os/unix/ngx_time.c


objs/src/os/unix/ngx_errno.o:	$(CORE_DEPS) \
	src/os/unix/ngx_errno.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_errno.o \
		src/os/unix/ngx_errno.c


objs/src/os/unix/ngx_alloc.o:	$(CORE_DEPS) \
	src/os/unix/ngx_alloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_alloc.o \
		src/os/unix/ngx_alloc.c


objs/src/os/unix/ngx_files.o:	$(CORE_DEPS) \
	src/os/unix/ngx_files.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_files.o \
		src/os/unix/ngx_files.c


objs/src/os/unix/ngx_socket.o:	$(CORE_DEPS) \
	src/os/unix/ngx_socket.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_socket.o \
		src/os/unix/ngx_socket.c


objs/src/os/unix/ngx_recv.o:	$(CORE_DEPS) \
	src/os/unix/ngx_recv.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_recv.o \
		src/os/unix/ngx_recv.c


objs/src/os/unix/ngx_readv_chain.o:	$(CORE_DEPS) \
	src/os/unix/ngx_readv_chain.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_readv_chain.o \
		src/os/unix/ngx_readv_chain.c


objs/src/os/unix/ngx_udp_recv.o:	$(CORE_DEPS) \
	src/os/unix/ngx_udp_recv.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_udp_recv.o \
		src/os/unix/ngx_udp_recv.c


objs/src/os/unix/ngx_send.o:	$(CORE_DEPS) \
	src/os/unix/ngx_send.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_send.o \
		src/os/unix/ngx_send.c


objs/src/os/unix/ngx_writev_chain.o:	$(CORE_DEPS) \
	src/os/unix/ngx_writev_chain.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_writev_chain.o \
		src/os/unix/ngx_writev_chain.c


objs/src/os/unix/ngx_channel.o:	$(CORE_DEPS) \
	src/os/unix/ngx_channel.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_channel.o \
		src/os/unix/ngx_channel.c


objs/src/os/unix/ngx_shmem.o:	$(CORE_DEPS) \
	src/os/unix/ngx_shmem.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_shmem.o \
		src/os/unix/ngx_shmem.c


objs/src/os/unix/ngx_process.o:	$(CORE_DEPS) \
	src/os/unix/ngx_process.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_process.o \
		src/os/unix/ngx_process.c


objs/src/os/unix/ngx_daemon.o:	$(CORE_DEPS) \
	src/os/unix/ngx_daemon.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_daemon.o \
		src/os/unix/ngx_daemon.c


objs/src/os/unix/ngx_setaffinity.o:	$(CORE_DEPS) \
	src/os/unix/ngx_setaffinity.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_setaffinity.o \
		src/os/unix/ngx_setaffinity.c


objs/src/os/unix/ngx_setproctitle.o:	$(CORE_DEPS) \
	src/os/unix/ngx_setproctitle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_setproctitle.o \
		src/os/unix/ngx_setproctitle.c


objs/src/os/unix/ngx_posix_init.o:	$(CORE_DEPS) \
	src/os/unix/ngx_posix_init.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_posix_init.o \
		src/os/unix/ngx_posix_init.c


objs/src/os/unix/ngx_user.o:	$(CORE_DEPS) \
	src/os/unix/ngx_user.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_user.o \
		src/os/unix/ngx_user.c


objs/src/os/unix/ngx_process_cycle.o:	$(CORE_DEPS) \
	src/os/unix/ngx_process_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_process_cycle.o \
		src/os/unix/ngx_process_cycle.c


objs/src/os/unix/ngx_linux_init.o:	$(CORE_DEPS) \
	src/os/unix/ngx_linux_init.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_linux_init.o \
		src/os/unix/ngx_linux_init.c


objs/src/event/modules/ngx_epoll_module.o:	$(CORE_DEPS) \
	src/event/modules/ngx_epoll_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/modules/ngx_epoll_module.o \
		src/event/modules/ngx_epoll_module.c


objs/src/os/unix/ngx_linux_sendfile_chain.o:	$(CORE_DEPS) \
	src/os/unix/ngx_linux_sendfile_chain.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/ngx_linux_sendfile_chain.o \
		src/os/unix/ngx_linux_sendfile_chain.c


objs/src/core/ngx_regex.o:	$(CORE_DEPS) \
	src/core/ngx_regex.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_regex.o \
		src/core/ngx_regex.c


objs/src/http/ngx_http.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http.o \
		src/http/ngx_http.c


objs/src/http/ngx_http_core_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_core_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_core_module.o \
		src/http/ngx_http_core_module.c


objs/src/http/ngx_http_special_response.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_special_response.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_special_response.o \
		src/http/ngx_http_special_response.c


objs/src/http/ngx_http_request.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_request.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_request.o \
		src/http/ngx_http_request.c


objs/src/http/ngx_http_parse.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_parse.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_parse.o \
		src/http/ngx_http_parse.c


objs/src/http/ngx_http_header_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_header_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_header_filter_module.o \
		src/http/ngx_http_header_filter_module.c


objs/src/http/ngx_http_write_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_write_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_write_filter_module.o \
		src/http/ngx_http_write_filter_module.c


objs/src/http/ngx_http_copy_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_copy_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_copy_filter_module.o \
		src/http/ngx_http_copy_filter_module.c


objs/src/http/modules/ngx_http_log_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_log_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_log_module.o \
		src/http/modules/ngx_http_log_module.c


objs/src/http/ngx_http_request_body.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_request_body.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_request_body.o \
		src/http/ngx_http_request_body.c


objs/src/http/ngx_http_variables.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_variables.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_variables.o \
		src/http/ngx_http_variables.c


objs/src/http/ngx_http_script.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_script.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_script.o \
		src/http/ngx_http_script.c


objs/src/http/ngx_http_upstream.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_upstream.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_upstream.o \
		src/http/ngx_http_upstream.c


objs/src/http/ngx_http_upstream_round_robin.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_upstream_round_robin.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_upstream_round_robin.o \
		src/http/ngx_http_upstream_round_robin.c


objs/src/http/ngx_http_parse_time.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_parse_time.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_parse_time.o \
		src/http/ngx_http_parse_time.c


objs/src/http/modules/ngx_http_static_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_static_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_static_module.o \
		src/http/modules/ngx_http_static_module.c


objs/src/http/modules/ngx_http_index_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_index_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_index_module.o \
		src/http/modules/ngx_http_index_module.c


objs/src/http/modules/ngx_http_chunked_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_chunked_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_chunked_filter_module.o \
		src/http/modules/ngx_http_chunked_filter_module.c


objs/src/http/modules/ngx_http_range_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_range_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_range_filter_module.o \
		src/http/modules/ngx_http_range_filter_module.c


objs/src/http/modules/ngx_http_headers_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_headers_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_headers_filter_module.o \
		src/http/modules/ngx_http_headers_filter_module.c


objs/src/http/modules/ngx_http_not_modified_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_not_modified_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_not_modified_filter_module.o \
		src/http/modules/ngx_http_not_modified_filter_module.c


objs/src/http/ngx_http_file_cache.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_file_cache.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_file_cache.o \
		src/http/ngx_http_file_cache.c


objs/src/http/modules/ngx_http_gzip_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_gzip_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_gzip_filter_module.o \
		src/http/modules/ngx_http_gzip_filter_module.c


objs/src/http/ngx_http_postpone_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_postpone_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_postpone_filter_module.o \
		src/http/ngx_http_postpone_filter_module.c


objs/src/http/modules/ngx_http_ssi_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_ssi_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_ssi_filter_module.o \
		src/http/modules/ngx_http_ssi_filter_module.c


objs/src/http/modules/ngx_http_charset_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_charset_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_charset_filter_module.o \
		src/http/modules/ngx_http_charset_filter_module.c


objs/src/http/modules/ngx_http_userid_filter_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_userid_filter_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_userid_filter_module.o \
		src/http/modules/ngx_http_userid_filter_module.c


objs/src/http/modules/ngx_http_autoindex_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_autoindex_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_autoindex_module.o \
		src/http/modules/ngx_http_autoindex_module.c


objs/src/http/modules/ngx_http_auth_basic_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_auth_basic_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_auth_basic_module.o \
		src/http/modules/ngx_http_auth_basic_module.c


objs/src/http/modules/ngx_http_access_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_access_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_access_module.o \
		src/http/modules/ngx_http_access_module.c


objs/src/http/modules/ngx_http_limit_conn_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_limit_conn_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_limit_conn_module.o \
		src/http/modules/ngx_http_limit_conn_module.c


objs/src/http/modules/ngx_http_limit_req_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_limit_req_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_limit_req_module.o \
		src/http/modules/ngx_http_limit_req_module.c


objs/src/http/modules/ngx_http_geo_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_geo_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_geo_module.o \
		src/http/modules/ngx_http_geo_module.c


objs/src/http/modules/ngx_http_map_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_map_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_map_module.o \
		src/http/modules/ngx_http_map_module.c


objs/src/http/modules/ngx_http_split_clients_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_split_clients_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_split_clients_module.o \
		src/http/modules/ngx_http_split_clients_module.c


objs/src/http/modules/ngx_http_referer_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_referer_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_referer_module.o \
		src/http/modules/ngx_http_referer_module.c


objs/src/http/modules/ngx_http_rewrite_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_rewrite_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_rewrite_module.o \
		src/http/modules/ngx_http_rewrite_module.c


objs/src/http/modules/ngx_http_proxy_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_proxy_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_proxy_module.o \
		src/http/modules/ngx_http_proxy_module.c


objs/src/http/modules/ngx_http_fastcgi_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_fastcgi_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_fastcgi_module.o \
		src/http/modules/ngx_http_fastcgi_module.c


objs/src/http/modules/ngx_http_uwsgi_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_uwsgi_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_uwsgi_module.o \
		src/http/modules/ngx_http_uwsgi_module.c


objs/src/http/modules/ngx_http_scgi_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_scgi_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_scgi_module.o \
		src/http/modules/ngx_http_scgi_module.c


objs/src/http/modules/ngx_http_memcached_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_memcached_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_memcached_module.o \
		src/http/modules/ngx_http_memcached_module.c


objs/src/http/modules/ngx_http_empty_gif_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_empty_gif_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_empty_gif_module.o \
		src/http/modules/ngx_http_empty_gif_module.c


objs/src/http/modules/ngx_http_browser_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_browser_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_browser_module.o \
		src/http/modules/ngx_http_browser_module.c


objs/src/http/modules/ngx_http_upstream_hash_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_upstream_hash_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_upstream_hash_module.o \
		src/http/modules/ngx_http_upstream_hash_module.c


objs/src/http/modules/ngx_http_upstream_ip_hash_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_upstream_ip_hash_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
		src/http/modules/ngx_http_upstream_ip_hash_module.c


objs/src/http/modules/ngx_http_upstream_least_conn_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_upstream_least_conn_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
		src/http/modules/ngx_http_upstream_least_conn_module.c


objs/src/http/modules/ngx_http_upstream_keepalive_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_upstream_keepalive_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
		src/http/modules/ngx_http_upstream_keepalive_module.c


objs/addon/lg_core/lg_config_file.o:	$(ADDON_DEPS) \
	src/lg_core/lg_config_file.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_config_file.o \
		src/lg_core/lg_config_file.c


objs/addon/lg_core/lg_db.o:	$(ADDON_DEPS) \
	src/lg_core/lg_db.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_db.o \
		src/lg_core/lg_db.c


objs/addon/lg_core/lg_list.o:	$(ADDON_DEPS) \
	src/lg_core/lg_list.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_list.o \
		src/lg_core/lg_list.c


objs/addon/lg_core/lg_ngx_network.o:	$(ADDON_DEPS) \
	src/lg_core/lg_ngx_network.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_ngx_network.o \
		src/lg_core/lg_ngx_network.c


objs/addon/lg_core/lg_ngx_session.o:	$(ADDON_DEPS) \
	src/lg_core/lg_ngx_session.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_ngx_session.o \
		src/lg_core/lg_ngx_session.c


objs/addon/lg_core/lg_ngx_upstream.o:	$(ADDON_DEPS) \
	src/lg_core/lg_ngx_upstream.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_ngx_upstream.o \
		src/lg_core/lg_ngx_upstream.c


objs/addon/lg_core/lg_ngx_uri.o:	$(ADDON_DEPS) \
	src/lg_core/lg_ngx_uri.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_ngx_uri.o \
		src/lg_core/lg_ngx_uri.c


objs/addon/lg_core/lg_ocilib.o:	$(ADDON_DEPS) \
	src/lg_core/lg_ocilib.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_ocilib.o \
		src/lg_core/lg_ocilib.c


objs/addon/lg_core/lg_rbtree.o:	$(ADDON_DEPS) \
	src/lg_core/lg_rbtree.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_rbtree.o \
		src/lg_core/lg_rbtree.c


objs/addon/lg_core/lg_string.o:	$(ADDON_DEPS) \
	src/lg_core/lg_string.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_core/lg_string.o \
		src/lg_core/lg_string.c


objs/addon/lg_json/json_array.o:	$(ADDON_DEPS) \
	src/lg_json/json_array.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/json_array.o \
		src/lg_json/json_array.c


objs/addon/lg_json/json_hash.o:	$(ADDON_DEPS) \
	src/lg_json/json_hash.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/json_hash.o \
		src/lg_json/json_hash.c


objs/addon/lg_json/json_object.o:	$(ADDON_DEPS) \
	src/lg_json/json_object.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/json_object.o \
		src/lg_json/json_object.c


objs/addon/lg_json/json_print.o:	$(ADDON_DEPS) \
	src/lg_json/json_print.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/json_print.o \
		src/lg_json/json_print.c


objs/addon/lg_json/json_tokener.o:	$(ADDON_DEPS) \
	src/lg_json/json_tokener.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/json_tokener.o \
		src/lg_json/json_tokener.c


objs/addon/lg_json/lg_json.o:	$(ADDON_DEPS) \
	src/lg_json/lg_json.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_json/lg_json.o \
		src/lg_json/lg_json.c


objs/addon/lg_ocilib/ocilib_agent.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_agent.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_agent.o \
		src/lg_ocilib/ocilib_agent.c


objs/addon/lg_ocilib/ocilib_array.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_array.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_array.o \
		src/lg_ocilib/ocilib_array.c


objs/addon/lg_ocilib/ocilib_bind.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_bind.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_bind.o \
		src/lg_ocilib/ocilib_bind.c


objs/addon/lg_ocilib/ocilib_callback.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_callback.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_callback.o \
		src/lg_ocilib/ocilib_callback.c


objs/addon/lg_ocilib/ocilib_collection.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_collection.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_collection.o \
		src/lg_ocilib/ocilib_collection.c


objs/addon/lg_ocilib/ocilib_column.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_column.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_column.o \
		src/lg_ocilib/ocilib_column.c


objs/addon/lg_ocilib/ocilib_connection.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_connection.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_connection.o \
		src/lg_ocilib/ocilib_connection.c


objs/addon/lg_ocilib/ocilib_date.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_date.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_date.o \
		src/lg_ocilib/ocilib_date.c


objs/addon/lg_ocilib/ocilib_define.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_define.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_define.o \
		src/lg_ocilib/ocilib_define.c


objs/addon/lg_ocilib/ocilib_dequeue.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_dequeue.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_dequeue.o \
		src/lg_ocilib/ocilib_dequeue.c


objs/addon/lg_ocilib/ocilib_dirpath.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_dirpath.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_dirpath.o \
		src/lg_ocilib/ocilib_dirpath.c


objs/addon/lg_ocilib/ocilib_element.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_element.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_element.o \
		src/lg_ocilib/ocilib_element.c


objs/addon/lg_ocilib/ocilib_enqueue.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_enqueue.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_enqueue.o \
		src/lg_ocilib/ocilib_enqueue.c


objs/addon/lg_ocilib/ocilib_error.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_error.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_error.o \
		src/lg_ocilib/ocilib_error.c


objs/addon/lg_ocilib/ocilib_event.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_event.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_event.o \
		src/lg_ocilib/ocilib_event.c


objs/addon/lg_ocilib/ocilib_exception.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_exception.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_exception.o \
		src/lg_ocilib/ocilib_exception.c


objs/addon/lg_ocilib/ocilib_file.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_file.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_file.o \
		src/lg_ocilib/ocilib_file.c


objs/addon/lg_ocilib/ocilib_format.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_format.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_format.o \
		src/lg_ocilib/ocilib_format.c


objs/addon/lg_ocilib/ocilib_handle.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_handle.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_handle.o \
		src/lg_ocilib/ocilib_handle.c


objs/addon/lg_ocilib/ocilib_hash.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_hash.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_hash.o \
		src/lg_ocilib/ocilib_hash.c


objs/addon/lg_ocilib/ocilib_interval.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_interval.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_interval.o \
		src/lg_ocilib/ocilib_interval.c


objs/addon/lg_ocilib/ocilib_iterator.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_iterator.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_iterator.o \
		src/lg_ocilib/ocilib_iterator.c


objs/addon/lg_ocilib/ocilib_library.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_library.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_library.o \
		src/lg_ocilib/ocilib_library.c


objs/addon/lg_ocilib/ocilib_list.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_list.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_list.o \
		src/lg_ocilib/ocilib_list.c


objs/addon/lg_ocilib/ocilib_lob.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_lob.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_lob.o \
		src/lg_ocilib/ocilib_lob.c


objs/addon/lg_ocilib/ocilib_long.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_long.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_long.o \
		src/lg_ocilib/ocilib_long.c


objs/addon/lg_ocilib/ocilib_memory.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_memory.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_memory.o \
		src/lg_ocilib/ocilib_memory.c


objs/addon/lg_ocilib/ocilib_msg.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_msg.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_msg.o \
		src/lg_ocilib/ocilib_msg.c


objs/addon/lg_ocilib/ocilib_mutex.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_mutex.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_mutex.o \
		src/lg_ocilib/ocilib_mutex.c


objs/addon/lg_ocilib/ocilib_number.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_number.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_number.o \
		src/lg_ocilib/ocilib_number.c


objs/addon/lg_ocilib/ocilib_object.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_object.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_object.o \
		src/lg_ocilib/ocilib_object.c


objs/addon/lg_ocilib/ocilib_pool.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_pool.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_pool.o \
		src/lg_ocilib/ocilib_pool.c


objs/addon/lg_ocilib/ocilib_queue.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_queue.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_queue.o \
		src/lg_ocilib/ocilib_queue.c


objs/addon/lg_ocilib/ocilib_ref.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_ref.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_ref.o \
		src/lg_ocilib/ocilib_ref.c


objs/addon/lg_ocilib/ocilib_resultset.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_resultset.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_resultset.o \
		src/lg_ocilib/ocilib_resultset.c


objs/addon/lg_ocilib/ocilib_statement.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_statement.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_statement.o \
		src/lg_ocilib/ocilib_statement.c


objs/addon/lg_ocilib/ocilib_string.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_string.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_string.o \
		src/lg_ocilib/ocilib_string.c


objs/addon/lg_ocilib/ocilib_subscription.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_subscription.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_subscription.o \
		src/lg_ocilib/ocilib_subscription.c


objs/addon/lg_ocilib/ocilib_thread.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_thread.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_thread.o \
		src/lg_ocilib/ocilib_thread.c


objs/addon/lg_ocilib/ocilib_threadkey.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_threadkey.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_threadkey.o \
		src/lg_ocilib/ocilib_threadkey.c


objs/addon/lg_ocilib/ocilib_timestamp.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_timestamp.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_timestamp.o \
		src/lg_ocilib/ocilib_timestamp.c


objs/addon/lg_ocilib/ocilib_transaction.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_transaction.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_transaction.o \
		src/lg_ocilib/ocilib_transaction.c


objs/addon/lg_ocilib/ocilib_typeinfo.o:	$(ADDON_DEPS) \
	src/lg_ocilib/ocilib_typeinfo.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/lg_ocilib/ocilib_typeinfo.o \
		src/lg_ocilib/ocilib_typeinfo.c


objs/addon/ad_system_display_module/ad_system_db_element.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_element.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_element.o \
		src/ad_system_display_module/ad_system_db_element.c


objs/addon/ad_system_display_module/ad_system_db_load.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_load.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_load.o \
		src/ad_system_display_module/ad_system_db_load.c


objs/addon/ad_system_display_module/ad_system_db_module.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_module.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_module.o \
		src/ad_system_display_module/ad_system_db_module.c


objs/addon/ad_system_display_module/ad_system_db_packet.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_packet.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_packet.o \
		src/ad_system_display_module/ad_system_db_packet.c


objs/addon/ad_system_display_module/ad_system_db_region_code.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_region_code.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_region_code.o \
		src/ad_system_display_module/ad_system_db_region_code.c


objs/addon/ad_system_display_module/ad_system_db_select.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/ad_system_db_select.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/ad_system_db_select.o \
		src/ad_system_display_module/ad_system_db_select.c


objs/addon/ad_system_display_module/lg_ip_addr_find.o:	$(ADDON_DEPS) \
	src/ad_system_display_module/lg_ip_addr_find.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/ad_system_display_module/lg_ip_addr_find.o \
		src/ad_system_display_module/lg_ip_addr_find.c


manpage:	objs/nginx.8

objs/nginx.8:	man/nginx.8 objs/ngx_auto_config.h
	sed -e "s|%%PREFIX%%|/usr/local/nginx|" \
		-e "s|%%PID_PATH%%|/ad_system/ad_system_familydoctor_yyk/conf/ad_system_display.pid|" \
		-e "s|%%CONF_PATH%%|/ad_system/ad_system_familydoctor_yyk/conf/ad_system_display.conf|" \
		-e "s|%%ERROR_LOG_PATH%%|/ad_system/ad_system_familydoctor_yyk/log/nginx-display/wangmeng.log|" \
		< man/nginx.8 > $@

install:	objs/nginx 		
	test -d '$(DESTDIR)/usr/local/nginx' || mkdir -p '$(DESTDIR)/usr/local/nginx'

	test -d '$(DESTDIR)/usr/local/nginx/sbin' 		|| mkdir -p '$(DESTDIR)/usr/local/nginx/sbin'
	test ! -f '$(DESTDIR)/usr/local/nginx/sbin/nginx' 		|| mv '$(DESTDIR)/usr/local/nginx/sbin/nginx' 			'$(DESTDIR)/usr/local/nginx/sbin/nginx.old'
	cp objs/nginx '$(DESTDIR)/usr/local/nginx/sbin/nginx'

	test -d '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf' 		|| mkdir -p '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'

	cp conf/koi-win '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/koi-utf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/win-utf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/mime.types' 		|| cp conf/mime.types '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/mime.types '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/mime.types.default'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/fastcgi_params' 		|| cp conf/fastcgi_params '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/fastcgi_params 		'$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/fastcgi_params.default'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/fastcgi.conf' 		|| cp conf/fastcgi.conf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/fastcgi.conf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/fastcgi.conf.default'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/uwsgi_params' 		|| cp conf/uwsgi_params '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/uwsgi_params 		'$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/uwsgi_params.default'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/scgi_params' 		|| cp conf/scgi_params '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'
	cp conf/scgi_params 		'$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/scgi_params.default'

	test -f '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/ad_system_display.conf' 		|| cp conf/nginx.conf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/ad_system_display.conf'
	cp conf/nginx.conf '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf/nginx.conf.default'

	test -d '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf' 		|| mkdir -p '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/conf'

	test -d '$(DESTDIR)/usr/local/nginx/logs' || 		mkdir -p '$(DESTDIR)/usr/local/nginx/logs'

	test -d '$(DESTDIR)/usr/local/nginx/html' 		|| cp -R html '$(DESTDIR)/usr/local/nginx'

	test -d '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/log/nginx-display' || 		mkdir -p '$(DESTDIR)/ad_system/ad_system_familydoctor_yyk/log/nginx-display'
