all: dell_service_tag.c
	cc --std=gnu99 -Wall -Wextra -O2 dell_service_tag.c -o dell_service_tag
