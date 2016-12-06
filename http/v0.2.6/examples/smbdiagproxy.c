/*
|  smbdiagproxy.c
|
|  EBS -
|
|   $Author:  $
|   $Date: 2016/08/22 19:14:06 $
|   $Name:  $
|   $Revision: 1.2 $
|
|  Copyright EBS Inc. , 20016
|  All rights reserved.
|  This code may not be redistributed in source or linkable object form
|  without the consent of its author.
*/

/*****************************************************************************/
/* Header files
 *****************************************************************************/

// #define CALL_SSL_MAIN 0

#include "rtpthrd.h"
#include "rtpnet.h"
#include "rtpterm.h"
#include "rtpprint.h"
#include "rtpstr.h"
#include "rtpscnv.h"
#include "httpsrv.h"
#include "htmlutils.h"

extern void diag_test(void);
extern int diag_recv_results_from_smb(char *buffer, int len);


 /* A context structure for keeping our application data
    This is a duplicate of the typedef in example_server.c
    this is bad form but not requiring an additional header file
    implifies the examples for documentation purposes. */
typedef struct s_HTTPExampleServerCtx
{
	HTTP_UINT8         rootDirectory[256];
	HTTP_UINT8         defaultFile[256];
  	HTTP_BOOL          disableVirtualIndex;
  	HTTP_BOOL          chunkedEncoding;
	HTTP_INT16         numHelperThreads;
	HTTP_INT16         numConnections;
	HTTP_UINT8         defaultIpAddr[4];
	HTTPServerConnection* connectCtxArray;
	HTTPServerContext  httpServer;
  	HTTP_BOOL          ModuleRequestingReload;
} HTTPExampleServerCtx;

static  HTTPExampleServerCtx ExampleServerCtx;

/*****************************************************************************/
/* Macros
 *****************************************************************************/

 /* Configuration parameters */

/* These values may be changed to customize the server. */
#define DEMO_SERVER_NAME "EBS WEB SERVER" /* Server name that we use to format the Server property in a standard html reponse header */
#define DEMO_WWW_ROOT "../../htdocs"        /* Path used as the root for all content loads from disk. */
#define DEMO_WWW_FILE "index.html"        /* The default file name to use if a user accesses the server providing no file name. Index.htm is recommended. */

static unsigned char
DEFAULT_DEMO_IPADDRESS[] = {0,0,0,0};   /* IP address of the interface to serve content to. Set to {0,0,0,0} to
                                           use the default interface. If you want to serve to an alternate interface
                                           set this to the IP addres of the alternate interface (for example: {192,168,1,1}.
                                           Note: Linux and windows do not but some emebedded network stack
                                           implementations may require this field to be initialized.  */

#define DEMO_MAX_CONNECTIONS 8		   /* Configure the maximum number of simultaneously connected/queued for acceptance */

#define DEMO_IPVERSION 4               /* Do not change. The API supports V6 but the demo is wired for V4 right now */
#define DEMO_MAX_HELPERS 1             /* Do not change. Number of helper threads to spawn. Must be 1. */




static int AjaxDemoValue=0;

 /*****************************************************************************/
/* Types
 *****************************************************************************/

/*****************************************************************************/
/* Function Prototypes
/*****************************************************************************/

/* Utility functions used by this example */
static int http_server_demo_restart(void);


HTTP_CHAR *http_demo_alloc_string(HTTP_CHAR *string);

/*****************************************************************************/
/* Data
 *****************************************************************************/

/*****************************************************************************/
/* Function Definitions
 *****************************************************************************/

#include<signal.h>
// #include <unistd.h>

volatile int go = 1; /* Variable loop on.. Note: Linux version needs sigkill support to clean up */

void sig_handler(int signo)
{
  if (signo == SIGINT)
  {
    go = 0;
  }
}

/*---------------------------------------------------------------------------*/

/* Main entry point for the web server application.
	A callable web server.
*/

int smbdiagproxy_server_main(void)
{
	HTTP_INT16 ipType = DEMO_IPVERSION;
	int idle_count = 0;
	int idle_count_minutes = 0;

    // Control C handler for setting go = 0
    signal(SIGINT, sig_handler);

	rtp_net_init();
	rtp_threads_init();

	/* Set initial default values */
	rtp_memset(&ExampleServerCtx, 0, sizeof(ExampleServerCtx));
	rtp_strcpy(ExampleServerCtx.rootDirectory, DEMO_WWW_ROOT);
	rtp_strcpy(ExampleServerCtx.defaultFile,DEMO_WWW_FILE);
	ExampleServerCtx.chunkedEncoding 		= 0;
	ExampleServerCtx.numHelperThreads 		= DEMO_MAX_HELPERS;
	ExampleServerCtx.numConnections		 	= DEMO_MAX_CONNECTIONS;
	/* If these are {0,0,0,0} use the default interface otherwise we use the configured address */
	ExampleServerCtx.defaultIpAddr[0] = DEFAULT_DEMO_IPADDRESS[0];
	ExampleServerCtx.defaultIpAddr[1] = DEFAULT_DEMO_IPADDRESS[1];
	ExampleServerCtx.defaultIpAddr[2] = DEFAULT_DEMO_IPADDRESS[2];
	ExampleServerCtx.defaultIpAddr[3] = DEFAULT_DEMO_IPADDRESS[3];

	rtp_printf("Using IP address %d.%d.%d.%d (all zeroes means use default interface) \n",
		ExampleServerCtx.defaultIpAddr[0],	ExampleServerCtx.defaultIpAddr[1],
		ExampleServerCtx.defaultIpAddr[2],	ExampleServerCtx.defaultIpAddr[3]);

	/* Initialize the server.
        Establish processing callbacks.
        Register memory based web pages.
	*/
	if (http_server_demo_restart() != 0)
		return(-1);


//========================
	/* Now loop continuously process one request per loop. */
	while (go)
	{
        int hits;
//		if (HTTP_ServerProcessOneRequest (&ExampleServerCtx.httpServer, 1000*60) < 0)
		if ((hits=HTTP_ServerProcessOneRequest (&ExampleServerCtx.httpServer, 100)) < 1)
		{
			/* Print an idle counter every minute the server is not accessed */
			idle_count += 1;
			if (idle_count == 100)
            {
				rtp_printf("\n Idle %d minutes      ", ++idle_count_minutes);
	            idle_count = 0;
            }
		}
		else
          idle_count = 0;

		if (ExampleServerCtx.ModuleRequestingReload)
		{
			ExampleServerCtx.ModuleRequestingReload = 0;
			HTTP_ServerDestroy(&ExampleServerCtx.httpServer, &ExampleServerCtx.connectCtxArray);
			rtp_free(ExampleServerCtx.connectCtxArray);
			ExampleServerCtx.connectCtxArray = 0;
			/* Initialize the server */
			if (http_server_demo_restart() != 0)
			{
				rtp_net_exit();
				return(-1);
			}
		}
	}

    rtp_printf("Bye !!\n");
    HTTP_ServerDestroy(&ExampleServerCtx.httpServer, &ExampleServerCtx.connectCtxArray);
	rtp_free(ExampleServerCtx.connectCtxArray);
	rtp_net_exit();
	return (0);

}


static int ajax_update_demo_cb (void *userData, HTTPServerRequestContext *ctx, HTTPSession *session, HTTPRequest *request, RTP_NET_ADDR *clientAddr);


/* Start or restart the server. Called when the application is first run and when callback handlers
   request a restart of the server because parametrs have been changed */
static int http_server_demo_restart(void)
{
	HTTP_INT16 ipType = DEMO_IPVERSION;	 /* The API supports V6 but the demo is wired for V4 right now */
	HTTP_UINT8     *pDefaultIpAddr;

	/* The demo uses 0,0,0,0 to signify use default but the API uses a null pointer so
	   map the demo methodology to the API metodology */
	if (ExampleServerCtx.defaultIpAddr[0] == 0 &&
		ExampleServerCtx.defaultIpAddr[1] == 0 &&
		ExampleServerCtx.defaultIpAddr[2] == 0 &&
		ExampleServerCtx.defaultIpAddr[3] == 0)
		pDefaultIpAddr = 0;
	else
		pDefaultIpAddr = &ExampleServerCtx.defaultIpAddr[0];

	rtp_printf("Starting or restarting server \n");

	/* Allocate and clear one server context per possible simultaneous connection. see (DEMO_MAX_CONNECTIONS) */
	if (!ExampleServerCtx.connectCtxArray)
	{
		ExampleServerCtx.connectCtxArray = (HTTPServerConnection*) rtp_malloc(sizeof(HTTPServerConnection) * ExampleServerCtx.numConnections);
		if (!ExampleServerCtx.connectCtxArray)
			return -1;
	}
	rtp_memset(ExampleServerCtx.connectCtxArray,0, sizeof(HTTPServerConnection) * ExampleServerCtx.numConnections);

	/* Initialize the server */
	if (HTTP_ServerInit (
			&ExampleServerCtx.httpServer,
			DEMO_SERVER_NAME, // name
			ExampleServerCtx.rootDirectory,    // rootDir
			ExampleServerCtx.defaultFile,      // defaultFile
			1,                // httpMajorVersion
			1,                // httpMinorVersion
			pDefaultIpAddr,
			8080, //8080,               // 80  ?? Use the www default port
			DEMO_IPVERSION,   // ipversion type(4 or 6)
			0,                // allowKeepAlive
			ExampleServerCtx.connectCtxArray,  // connectCtxArray
			ExampleServerCtx.numConnections,   // maxConnections
			ExampleServerCtx.numHelperThreads  // maxHelperThreads
		) < 0)
		return -1;

	/* Create an ajax applet that accepts value updates from a browser via POST and returns a modified
       value (multiplied by 4) in a reply.
       The client applications Html5Demo.html and Html4Demo.html use the reply data  from "\\smbdiag_ajax_getval"
       to update their screens.
	*/
     /*  Add a url that updates a value stored in the variable AjaxDemoValue. The value is poste by the Html5Demo.html example when the slider is moved */
	if (HTTP_ServerAddPostHandler(&ExampleServerCtx.httpServer, "\\smbdiag_ajax_setval_submit", ajax_update_demo_cb, (void *)"set_value") < 0)
		return -1;
     /*  Add a url that updates a value stored in the variable AjaxDemoValue. The value is posted by the Html4Demo.html example when a table cell is clicked. */
	if (HTTP_ServerAddPostHandler(&ExampleServerCtx.httpServer, "\\smbdiag_ajax_command_submit", ajax_update_demo_cb, (void *)"process_command") < 0)
		return -1;
     /*  Add a url that returns the current tick count and the current value stored in the variable AjaxDemoValue. Html4Demo.html and Html5Demo.html retrieve these
         an update their screens. */
	if (HTTP_ServerAddPostHandler(&ExampleServerCtx.httpServer, "\\smbdiag_ajax_getval", ajax_update_demo_cb, (void *)"get_value") < 0)
		return -1;
	rtp_printf("Post Handlers for smbdiag_ajax_setval_submit, smbdiag_ajax_command_submit and smbdiag_ajax_getval have been assigned\n");

	if (HTTP_ServerAddPostHandler(&ExampleServerCtx.httpServer, "\\smbdiag_ajax_getfids", ajax_update_demo_cb, (void *)"get_value") < 0)
		return -1;
	return (0);
}



/*---------------------------------------------------------------------------*/
int             _HTTP_ServerHeaderCallback      (void *userData,
                                                 HTTPSession *session,
                                                 HTTPHeaderType type,
                                                 const HTTP_CHAR *name,
                                                 const HTTP_CHAR *value);



static int demo_form_cb (void *userData, HTTPServerRequestContext *ctx, HTTPSession *session, HTTPRequest *request, RTP_NET_ADDR *clientAddr)
{
int processing_configure = 0;
HTTPServerContext *server = ctx->server;
HTTP_CHAR formName[256];

	{
		HTTP_CHAR *p;
		p = rtp_strstr(request->target, "?");
		if (p)
		{
			rtp_strncpy(formName, request->target, (int)(p-request->target));
			formName[(int) (p-request->target)]=0;
		}
		else
		{
			rtp_strcpy(formName, request->target);
		}
	}
	rtp_printf("formName from URL == %s\n", formName);
	rtp_printf("user data  == %s\n", userData);


	switch (request->methodType)
	{
		case HTTP_METHOD_POST:
        {
			HTTPResponse response;
			HTTP_INT32 bytesRead=-1;
			HTTP_UINT8 tempBufferRaw[256];
				if (HTTP_ReadHeaders(session, _HTTP_ServerHeaderCallback, ctx, tempBufferRaw,sizeof(tempBufferRaw)) >= 0)
				{
					bytesRead = HTTP_Read(session,tempBufferRaw,sizeof(tempBufferRaw));
				}
				HTTP_ServerInitResponse(ctx, session, &response, 200, "OK");
				HTTP_ServerSetDefaultHeaders(ctx, &response);
				HTTP_SetResponseHeaderStr(&response, "Content-Type", "text/html");
				HTTP_WriteResponse(session, &response);
				{
				HTTP_UINT8 Buffer[64];
				rtp_sprintf(Buffer,"Received a post of %d bytes.<br>Value pairs are:<br>",bytesRead);
				HTTP_Write(session, Buffer, rtp_strlen(Buffer));
				// if (bytesRead > 0)
				//	 HTTP_Write(session, tempBufferRaw, bytesRead);
                // ==========================
                // Display name value pairs of posted data
				if (bytesRead > 0)
				{
					NVPairList PairList;
                    NVPair PairArray[10];
                    int n,escapedlen,consumed;
					HTTP_UINT8 tempBuffer[256];
					tempBufferRaw[bytesRead]=0;

					/* & characters are converted to zeroes for the convenience of name value pair parser. */
					escapedlen = HTML_UnEscapeFormString(tempBuffer,tempBufferRaw);
                    HTTP_InitNameValuePairList (&PairList, PairArray, 10);
                    consumed = HTTP_LoadNameValuePairs (tempBuffer, escapedlen, &PairList);
                    for (n=0; HTTP_GetNameValuePairIndexed (&PairList,n); n++)
                    {
					    HTTP_UINT8 *name = (HTTP_UINT8 *)  HTTP_GetNameValuePairIndexed(&PairList,n)->name;
					    HTTP_UINT8 *value = (HTTP_UINT8 *) HTTP_GetNameValuePairIndexed(&PairList,n)->value;
    					if (name && value)
    					{
						HTTP_UINT8 respBuffer[256];
    						if (processing_configure)
    						{
    							if (rtp_strcmp(name, "root") == 0)
    							{
    								rtp_strcpy(ExampleServerCtx.rootDirectory, value);
    								ExampleServerCtx.ModuleRequestingReload = 1;
    							}
    						}
    						rtp_strcpy(respBuffer, name);
    						rtp_strcat(respBuffer, " = ");
    						rtp_strcat(respBuffer, value);
    						rtp_strcat(respBuffer, " <br>");
							HTTP_Write(session, respBuffer, rtp_strlen(respBuffer));
    					}
    				}
                }
                // ==========================

				}
				HTTP_WriteFlush(session);
				HTTP_FreeResponse(&response);
				return (HTTP_REQUEST_STATUS_DONE);
                break;
        }
		case HTTP_METHOD_GET:
		{
			HTTPResponse response;
			HTTP_UINT8 tempBuffer[256];
			HTTP_INT32 tempBufferSize = 256;
			HTTP_UINT8 *respBuffer;
			HTTP_INT32 respBufferSize = 4096;

			HTTP_CHAR *p;
			int pairs = 0;

			if (rtp_strcmp("demo_configure_submit", (char *) userData) == 0)
			{
				processing_configure = 1;
			}

			/* Allocate a buffer for sending response. */
			respBuffer = (HTTP_UINT8 *) rtp_malloc(respBufferSize);
			*respBuffer = 0;
			/* Parse the submittd values from the command line and send them back to the user in a document */
			p = rtp_strstr(request->target, "?");
			if (p)
			{
			HTTP_UINT8 *name;
			HTTP_UINT8 *value;
			NVPairList PairList;
            NVPair PairArray[10];
            PNVPair pSetValPair;
            int escapedlen,consumed,n;

				p++;
			    /* & characters are converted to zeroes for the convenience of name value pair parser. */
				escapedlen = HTML_UnEscapeFormString(tempBuffer,p);
                HTTP_InitNameValuePairList (&PairList, PairArray, ARRAYSIZE(PairArray));
                consumed = HTTP_LoadNameValuePairs (tempBuffer, escapedlen, &PairList);

                pSetValPair=HTTP_GetNameValuePairAssociative (&PairList,"AjaxSetVal");

				rtp_strcat(respBuffer, "<hr>You submitted these values: <hr>");
                 pSetValPair=HTTP_GetNameValuePairIndexed (&PairList,0);
				for (n=0; HTTP_GetNameValuePairIndexed (&PairList,n); n++)
				{
					name = (HTTP_UINT8 *)  HTTP_GetNameValuePairIndexed(&PairList,n)->name;
                    value = (HTTP_UINT8 *) HTTP_GetNameValuePairIndexed(&PairList,n)->value;
					if (name && value)
					{
						if (processing_configure)
						{
							if (rtp_strcmp(name, "root") == 0)
							{
								rtp_strcpy(ExampleServerCtx.rootDirectory, value);
								ExampleServerCtx.ModuleRequestingReload = 1;
							}
						}
						rtp_strcat(respBuffer, name);
						rtp_strcat(respBuffer, " = ");
						rtp_strcat(respBuffer, value);
						rtp_strcat(respBuffer, "<br>");
					}
				}
			}

			/* Read the HTTP headers into an NV pair list. Dynamically allocate 2048 bytes to hold the header contents.
               This memory block is structured and the Headers object is populated with null terminated header name value pairs.
			   To reduce memory requirements a filter function can be passed in that selectively decides whether to keep elements.
			*/
            {
                NVHeaderList Headers;
                int n,HeaderBufferSize;
                HTTP_UINT8 *  HeaderBuffer = (HTTP_UINT8 *) rtp_malloc(2048);
                HeaderBufferSize = 2048;

                HTTP_InitHeaderNameValuePairList ( &Headers,HeaderBuffer,HeaderBufferSize,0,0);
                HTTP_ServerReadHeaders(	ctx, session, HTTP_CallbackHeaderToNameValuePairList, &Headers, tempBuffer, tempBufferSize);

                rtp_sprintf(respBuffer+rtp_strlen(respBuffer), "<hr>Your browser passed these HTTP header values in %d bytes: <hr>", Headers.BufferUsed);
				for (n=0; HTTP_GetNameValuePairIndexed (&Headers.PairList,n); n++)
				{
				    HTTP_CHAR *name = (HTTP_UINT8 *)  HTTP_GetNameValuePairIndexed(&Headers.PairList,n)->name;
				    HTTP_CHAR *value = (HTTP_UINT8 *) HTTP_GetNameValuePairIndexed(&Headers.PairList,n)->value;
					if (name && value)
					{
                        rtp_strcat(respBuffer, name);
                        rtp_strcat(respBuffer, " = ");
                        rtp_strcat(respBuffer, value);
                        rtp_strcat(respBuffer, "<br>");
					}
				}
				rtp_free(HeaderBuffer);
            }

			if (processing_configure)
				rtp_strcat(respBuffer, "<hr>The configruation was changed. To test your results select the option to revert to disk based html from the main page.<br><br>");
			rtp_strcat(respBuffer, "<hr>Thank you ... and have a nice day<hr>");
			rtp_strcat(respBuffer, "<br><a href=\"romindex.htm\">Click here to return to main index.</a><br>");

			HTTP_ServerInitResponse(ctx, session, &response, 200, "OK");
			HTTP_ServerSetDefaultHeaders(ctx, &response);
			HTTP_SetResponseHeaderStr(&response, "Content-Type", "text/html");

			HTTP_SetResponseHeaderInt(&response, "Content-Length", rtp_strlen(respBuffer));

			HTTP_WriteResponse(session, &response);
			HTTP_Write(session, respBuffer, rtp_strlen(respBuffer));
			HTTP_WriteFlush(session);

			rtp_free(respBuffer);
			HTTP_FreeResponse(&response);

			return (HTTP_REQUEST_STATUS_DONE);
		}

		case HTTP_METHOD_HEAD: /* tbd - implement HEAD method */
		default:
			return (HTTP_REQUEST_STATUS_CONTINUE);
	}
}

int MyHeaderCallback(
		void *userData,
		HTTPServerRequestContext *ctx,
		HTTPSession *session,
		HTTPHeaderType type,
		const HTTP_CHAR *name,
		const HTTP_CHAR *value
	)
{
	printf("Name == %s: Value ==:%s\n", name, value);
	return 0;
}
#define TEST_JSON 0
#if (TEST_JSON)
#define MAXVALS 200
static int value_array[MAXVALS];
static int value_count=0;

static void log_json_val(int new_time,int new_val)
{
   if (value_count < MAXVALS)
     value_array[value_count++] = new_val;
}

static void formatjson_val( char *buff)
{
  int index;
  char *b= buff;
//    b += rtp_sprintf(b,"<html><body><b>Systick [%d] <br><b>",rtp_get_system_msec(), AjaxDemoValue);
  	b += rtp_sprintf(b,"[");
    if(1)
    {
    for (index = 0; index < 400; index++)
	{
      int i = (int) 128*sin(index*0.0174532925);
      b += rtp_sprintf(b,"{\"x\":%d, \"y\":%d}",index, i);
      if (index < 400)
        b += rtp_sprintf(b,",");
//	  HTTP_Write(session, buff, rtp_strlen(buff));
	}
    }
    if(0)
    for (index = 0; index < value_count; index++)
	{
      b += rtp_sprintf(b,"{\"x\":%d, \"y\":%d}", index,value_array[index]);
	  if (index < value_count)
		  b += rtp_sprintf(b,",");
//	  HTTP_Write(session, buff, rtp_strlen(buff));
	}
  	b += rtp_sprintf(b,"]");
//    b += rtp_sprintf(b,"</b></body></html>",rtp_get_system_msec(), AjaxDemoValue);
}

#endif
static int ajax_update_demo_cb (void *userData, HTTPServerRequestContext *ctx, HTTPSession *session, HTTPRequest *request, RTP_NET_ADDR *clientAddr)
{
int processing_configure = 0;
HTTPServerContext *server = ctx->server;
HTTP_CHAR formName[256];
HTTP_CHAR cgiArgs[256];
    cgiArgs[0]=0;
	{
		HTTP_CHAR *p;
		p = rtp_strstr(request->target, "?");
		if (p)
		{
			rtp_strncpy(formName, request->target, (int)(p-request->target));
			formName[(int)(p-request->target)]=0;
			rtp_strcpy(cgiArgs, p+1);
		}
		else
		{
			rtp_strcpy(formName, request->target);
		}
	}
	rtp_printf("request->target  == %s\n", request->target);
	rtp_printf("formName from URL == %s\n", formName);
	rtp_printf("user date  == %s\n", userData);
	rtp_printf("cgiArgs  == %s\n", cgiArgs);

	switch (request->methodType)
	{
		case HTTP_METHOD_GET:
		{
			HTTPResponse response;
			HTTP_UINT8 *respBuffer;
			HTTP_INT32 respBufferSize = 40960;

			{
			HTTP_UINT8 tempBufferRaw[256];
			ctx->userCallback = MyHeaderCallback;
			printf("==========================================================================================\n");
			HTTP_ReadHeaders(session, _HTTP_ServerHeaderCallback, ctx, tempBufferRaw,sizeof(tempBufferRaw));
			printf("==========================================================================================\n");
			}

			/* Allocate a buffer for sending response. */
			respBuffer = (HTTP_UINT8 *) rtp_malloc(respBufferSize);
			*respBuffer = 0;
#if (TEST_JSON)
     		if (value_count > 100)
            {
			   formatjson_val(respBuffer);
               value_count = 0;
            }
#else
    if (rtp_strcmp(request->target, "/smbdiag_ajax_getfids")==0)
    {
    int l,j;
        diag_send_command("SMB FIDS");
        j = diag_recv_results_from_smb(respBuffer, respBufferSize);
        if (j > 0)
			respBuffer[j]=0;
        else
            strcpy(respBuffer,"Failed retrieving a response from the server");
    }
    else // if (rtp_strcmp(request->target, "/smbdiag_ajax_getval")==0)
    {
			sprintf(respBuffer,"Command %s is not implement yet <br>",request->target);
    }
#endif
			HTTP_ServerInitResponse(ctx, session, &response, 200, "OK");
			HTTP_ServerSetDefaultHeaders(ctx, &response);
			HTTP_SetResponseHeaderStr(&response, "Content-Type", "text/html");
			HTTP_SetResponseHeaderInt(&response, "Content-Length", rtp_strlen(respBuffer));
			HTTP_WriteResponse(session, &response);
			HTTP_Write(session, respBuffer, rtp_strlen(respBuffer));
			HTTP_WriteFlush(session);
			rtp_free(respBuffer);
			HTTP_FreeResponse(&response);
			return (HTTP_REQUEST_STATUS_DONE);
		}
		case HTTP_METHOD_POST:
        {
			HTTP_INT32 bytesRead;
			HTTP_UINT8 tempBufferRaw[256];
			HTTP_UINT8 tempBuffer[256];
			ctx->userCallback = MyHeaderCallback;
			if (HTTP_ReadHeaders(session, _HTTP_ServerHeaderCallback, ctx, tempBufferRaw,sizeof(tempBufferRaw)) >= 0)
			{

				bytesRead = HTTP_Read(session,tempBufferRaw,sizeof(tempBufferRaw));
				if (bytesRead > 0)
				{
					NVPairList PairList;
                    NVPair PairArray[10];
                    PNVPair pSetValPair;
                    int escapedlen,consumed;
					tempBufferRaw[bytesRead]=0;
					/* & characters are converted to zeroes for the convenience of name value pair parser. */
					escapedlen = HTML_UnEscapeFormString(tempBuffer,tempBufferRaw);
                    HTTP_InitNameValuePairList (&PairList, PairArray, 10);
                    consumed = HTTP_LoadNameValuePairs (tempBuffer, escapedlen, &PairList);

                    pSetValPair=HTTP_GetNameValuePairAssociative (&PairList,"AjaxSetVal");

					if (pSetValPair)
					{
                    char *psmbcmd=0;
						if (userData && rtp_strcmp((HTTP_CHAR *)userData, "process_command")==0)
						{
						    if (rtp_strcmp(pSetValPair->value, "clear")==0)
                            {
                                diag_test();
							    AjaxDemoValue = 0;
                            }
							else if (rtp_strcmp(pSetValPair->value, "+10")==0)
							    AjaxDemoValue +=10;
							else if (rtp_strcmp(pSetValPair->value, "-10")==0)
							    AjaxDemoValue -=10;
							else if (rtp_strcmp(pSetValPair->value, "+100")==0)
							    AjaxDemoValue +=100;
							else if (rtp_strcmp(pSetValPair->value, "-100")==0)
							    AjaxDemoValue -=100;
                        }
						else if (userData && rtp_strcmp((HTTP_CHAR *)userData, "set_value")==0)
                        {
							AjaxDemoValue=rtp_atol(pSetValPair->value);
                        }
                        if ((psmbcmd=rtp_strstr(pSetValPair->value, "SMB")))
                        {
                            rtp_printf("Send %s\n", psmbcmd);
                            diag_test();
                            AjaxDemoValue = 0;
                        }
						rtp_printf("Changed V to %d\n", AjaxDemoValue);
#if (TEST_JSON)
						log_json_val(0,AjaxDemoValue);
#endif
					}
				}

				{
					HTTPResponse response;
					HTTP_ServerInitResponse(ctx, session, &response, 201, "OK");
					HTTP_WriteResponse(session, &response);
					{
					char buff[80];
						rtp_sprintf(buff,"%d\n",AjaxDemoValue*4);
					HTTP_Write(session, buff, rtp_strlen(buff));
					}
//					HTTP_Write(session, tempBufferRaw, rtp_strlen(tempBufferRaw));
					HTTP_WriteFlush(session);
					HTTP_FreeResponse(&response);
				}
			}
            return (HTTP_REQUEST_STATUS_DONE);
       }
		default:
			return (HTTP_REQUEST_STATUS_DONE);
    }
}
