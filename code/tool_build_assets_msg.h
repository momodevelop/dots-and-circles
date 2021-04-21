/* date = April 21st 2021 6:01 pm */

#ifndef TOOL_BUILD_ASSETS_MSG_H
#define TOOL_BUILD_ASSETS_MSG_H

struct tba_message_context {
    msg_id Id;
    const char* Str;
};

static  tba_message_context
Tba_MessageContexts[] = {
    Msg_ConsoleJumpToMain, "Jumping to Main",
    Msg_ConsoleJumpToMenu, "Jumping to Menu",
    Msg_ConsoleJumpToSandbox, "Jumping to Sandbox",
};



#endif //TOOL_BUILD_ASSETS_MSG_H
