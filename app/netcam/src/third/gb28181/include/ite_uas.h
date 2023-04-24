
#ifndef ITE_UAS_H
#define ITE_UAS_H

#include "ite_sip_api.h"
#include "ite_sip_interface.h"
#include <pthread.h>


int handle_invite(struct eXosip_t *excontext,eXosip_event_t * g_event)
{
    //int length;
    char * message;
    //osip_message_to_str(g_event->request, &message, &length);
    osip_body_t *body;
    osip_message_get_body (g_event->request, 0, &body);
    message = (char *)malloc (body->length * sizeof(char));
    snprintf (message, body->length, "%s", body->body);

    //interface do somthing with the sdp
    //function_run(uas_handle_invite_sdp,message);
    //uas_function_run(uas_handle_invite_sdp, message);
    //end interface


    if(MSG_IS_INVITE(g_event->request))
    {
        osip_message_t *asw_msg = NULL;

        eXosip_lock(excontext);
        if(0 != eXosip_call_build_answer(excontext,g_event->tid, 200, &asw_msg))/*Build default Answer for request*/
        {
            eXosip_call_send_answer(excontext,g_event->tid, 603, NULL);
            eXosip_unlock(excontext);
            printf("eXosip_call_build_answer error!\r\n");
            return -1;
        }
        eXosip_unlock(excontext);

        char sdp_body[1024];
        //char sdp_body[4096];
        //memset(sdp_body, 0, 4096);
        //printf("<MSG_IS_INVITE>\r\n");

        //interface get sdp
        uas_function_run(uas_get_invite_sdp, sdp_body);
        //end interface

        eXosip_lock(excontext);
        osip_message_set_body(asw_msg, sdp_body, strlen(sdp_body));
        osip_message_set_content_type(asw_msg, "application/sdp");
        eXosip_call_send_answer(excontext,g_event->tid, 200, asw_msg);
        printf("eXosip_call_send_answer success!\r\n");
        eXosip_unlock(excontext);

        //interface get sdp
        uas_function_run(uas_start_transport, NULL);
        //end interface
    }
    return 0;
}

int handle_message(struct eXosip_t *excontext,eXosip_event_t * g_event)
{
    osip_body_t *msg_body = NULL;
    osip_message_t *g_answer = NULL;
    //printf("<MSG_IS_INFO>\r\n");
    osip_message_get_body(g_event->request, 0, &msg_body);
    if(NULL != msg_body)
    {
        eXosip_call_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
        eXosip_call_send_answer(excontext,g_event->tid, 200, g_answer);
        printf("eXosip_call_send_answer success!\r\n");
        //ite_eXosip_paraseInfoBody(g_event);
        char * message;
        osip_body_t *body;
        osip_message_get_body (g_event->request, 0, &body);
        message = (char *)malloc (body->length * sizeof(char));
        snprintf (message, body->length, "%s", body->body);

        //interface handle the message
        uas_function_run(uas_handle_Message, message);
        //end interface

        free(message);
    }

    return 0;
}

int handle_bye(struct eXosip_t *excontext,eXosip_event_t * g_event)
{
    printf("\r\n<EXOSIP_CALL_CLOSED>\r\n");
    if(MSG_IS_BYE(g_event->request))
    {
#if 0
        printf("<MSG_IS_BYE>\r\n");
        if((0 != g_did_realPlay) && (g_did_realPlay == g_event->did))
        {
            printf("realPlay closed success!\r\n");

            //interface stop transport
            uas_function_run(uas_stop_transport, NULL);
            //end interface

            g_did_realPlay = 0;
        }
        else if((0 != g_did_backPlay) && (g_did_backPlay == g_event->did))
        {
            printf("backPlay closed success!\r\n");

            //interface stop transport
            uas_function_run(uas_stop_transport, NULL);
            //end interface

            g_did_backPlay = 0;
        }
        else if((0 != g_did_fileDown) && (g_did_fileDown == g_event->did))
        {
            printf("fileDown closed success!\r\n");

            //interface stop transport
            uas_function_run(uas_stop_transport, NULL);
            //end interface

            g_did_fileDown = 0;
        }
        if((0 != g_call_id) && (0 == g_did_realPlay) && (0 == g_did_backPlay) && (0 == g_did_fileDown))
        {
            printf("call closed success!\r\n");
            g_call_id = 0;
        }
#endif
    }
    return 0;
}

void uas_eXosip_processEvent(struct eXosip_t *excontext)
{
    eXosip_event_t *g_event  = NULL;
    osip_message_t *g_answer = NULL;
    while (1)
    {
        g_event = eXosip_event_wait(excontext,0, 200);
        eXosip_lock(excontext);
        eXosip_default_action(excontext,g_event);
       // eXosip_automatic_refresh(excontext);/*Refresh REGISTER and SUBSCRIBE before the expiration delay*/
        eXosip_unlock(excontext);
        if (NULL == g_event)
        {
            continue;
        }
        //ite_eXosip_printEvent(g_event);
        switch (g_event->type)
        {
            case EXOSIP_MESSAGE_NEW:/*MESSAGE:MESSAGE*/
            {
                printf("\r\n<EXOSIP_MESSAGE_NEW>\r\n");
                if(MSG_IS_MESSAGE(g_event->request))
                {
                    printf("<MSG_IS_MESSAGE>\r\n");
                    eXosip_lock(excontext);
                    eXosip_message_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
                    eXosip_message_send_answer(excontext,g_event->tid, 200, g_answer);
                    printf("eXosip_message_send_answer success!\r\n");
                    eXosip_unlock(excontext);
                    //ite_eXosip_ProcessMsgBody(excontext,g_event);
                }
            }
            break;
            case EXOSIP_MESSAGE_ANSWERED:/*200OK*/
            {
                printf("\r\n<EXOSIP_MESSAGE_ANSWERED>\r\n");
            }
            break;
            case EXOSIP_CALL_INVITE:/*INVITE*/
            {
                printf("\r\n<EXOSIP_CALL_INVITE>\r\n");

                handle_invite(excontext,g_event);
            }
            break;
            case EXOSIP_CALL_ACK:/*ACK*/
            {

                printf("\r\n<EXOSIP_CALL_ACK>\r\n");
               // ite_eXosip_parseInviteBody(excontext,g_event,SipConfig);
            }
            break;
            case EXOSIP_CALL_CLOSED:/*BEY*/
            {
                handle_bye(excontext,g_event);
            }
            break;
            case EXOSIP_CALL_MESSAGE_NEW:/*MESSAGE:INFO*/
            {
                printf("\r\n<EXOSIP_CALL_MESSAGE_NEW>\r\n");
                if(MSG_IS_INFO(g_event->request) || MSG_IS_MESSAGE(g_event->request)) //identify the info package
                {
                    handle_message(excontext,g_event);
                }
            }
            break;
            case EXOSIP_CALL_MESSAGE_ANSWERED:/*200OK*/
            {
                printf("\r\n<EXOSIP_CALL_MESSAGE_ANSWERED>\r\n");
            }
            break;
            default:
            {
                printf("\r\n<OTHER>\r\n");
                printf("eXosip event type:%d\n", g_event->type);
            }
            break;
        }
    }
}

#endif //ITE_UAS_H

