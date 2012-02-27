#include "epollServer.h"

#include <signal.h>
#include <unistd.h>

int main() {

    blockSignals();
    startServer();
    //alarm(1); //sets the first timer for metrics.
    return 0;

}


