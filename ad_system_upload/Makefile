
SRCPATH = src/
OBJPATH = obj/
CFLAG = -Wall -g
INCLUDE = -I./include -I./include/libgd
LIBPATH = -ljpeg -lpng -lgd -pthread -luuid -lhiredis


UPLOAD_KEYWORD_OBJ += $(OBJPATH)json_array.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)json_hash.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)json_object.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)json_print.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)json_tokener.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)lg_string.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)lg_string_json.o
UPLOAD_KEYWORD_OBJ += $(OBJPATH)ad_system_upload_keyword.o

UPLOAD_IMAGE_OBJ = $(OBJPATH)ad_system_upload_image.o
UPLOAD_MANAGER_IMAGE_OBJ = $(OBJPATH)ad_system_manager_upload_image.o

all:clean $(UPLOAD_KEYWORD_OBJ) $(UPLOAD_IMAGE_OBJ) $(UPLOAD_MANAGER_IMAGE_OBJ)
	gcc $(INCLUDE) $(UPLOAD_KEYWORD_OBJ) $(LIBPATH) $(CFLAG) -o ad_system_upload_keyword
	gcc $(INCLUDE) $(UPLOAD_IMAGE_OBJ) $(LIBPATH) $(CFLAG) -o ad_system_upload_image
	gcc $(INCLUDE) $(UPLOAD_MANAGER_IMAGE_OBJ) $(LIBPATH) $(CFLAG) -o ad_system_manager_upload_image

$(OBJPATH)%.o:$(SRCPATH)%.c
	gcc $(INCLUDE) -c $< -o $@ $(CFLAG)

clean:
	rm -f $(UPLOAD_KEYWORD_OBJ)
	rm -f $(UPLOAD_IMAGE_OBJ)
	rm -f $(UPLOAD_MANAGER_IMAGE_OBJ)
	rm -f ad_system_upload_keyword
	rm -f ad_system_upload_image
	rm -f ad_system_manager_upload_image
	

