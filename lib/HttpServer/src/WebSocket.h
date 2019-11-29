#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "Task.h"
#include "esp_log.h"
#include "sdkconfig.h"


namespace server {

class WebSocket : public Task
{
public:
    WebSocket();
    virtual ~WebSocket();



    virtual void run(void* data) override
    {
        while (1)
            ;
    }

private:
};

} // namespace server
#endif