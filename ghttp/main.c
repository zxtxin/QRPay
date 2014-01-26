#include "ghttp.h"
#include <stdio.h>



void main(int argc,char *argv[])
{
	ghttp_request *request = NULL;
	request = ghttp_request_new();
	ghttp_set_uri(request, argv[1]);
	ghttp_set_type(request, ghttp_type_post);
	ghttp_set_body(request, argv[2],strlen(argv[2]));
	printf("%s\n", argv[2]);

	ghttp_prepare(request);
	ghttp_process(request);
	fwrite(ghttp_get_body(request), ghttp_get_body_len(request), 1, stdout);
	fprintf(stdout, "  Length: %s\n", ghttp_get_header(request, "Content-Length"));
	fprintf(stdout, "  Location: %s\n", ghttp_get_header(request, "Location"));
	
}


