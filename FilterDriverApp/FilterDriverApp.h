#pragma once

#include "resource.h"
#include "Commctrl.h"
#include "Setupapi.h"
#include "combaseapi.h"
#include "Netcfgx.h"

enum myID {
    ID_LISTBOX_1,
    ID_LISTBOX_2,
    ID_TEXTBOX_1,
    ID_TEXTBOX_2,
    ID_BUTTON_DELETE_1,
    ID_BUTTON_DELETE_2,
    ID_BUTTON_ADD_1,
    ID_BUTTON_ADD_2,
    ID_BUTTON_CLEAR_1,
    ID_BUTTON_CLEAR_2,
    ID_LABEL_1,
    ID_LABEL_2
};

typedef enum {
    ADD_RECEIVE_IP,
    DELETE_RECEIVE_IP,
    CLEAR_RECEIVE_IP,
    ADD_SEND_IP,
    DELETE_SEND_IP,
    CLEAR_SEND_IP
} TYPE_CHANGE;

typedef struct block_list {
    struct block_list* next;
    UINT32 ip;
} block_list_t;

typedef struct {
    UINT32 IP;
    TYPE_CHANGE type;
} CHANGE_LIST_IP_INFO;