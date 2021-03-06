#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

static const char QUEUE_PATH[] = "/tmp";

int InitializeMessageQueue(int queueId)
{
    // get queue key
    key_t queueKey;
    queueKey = ftok(QUEUE_PATH, queueId);
    if(queueKey == -1)
    {
        switch (errno)
        {
            case EACCES:
                printf("Getting queue key failed: Search permission is denied for a component of the path prefix.\n");
                break;
            case ENOENT:
                printf("Getting queue key failed: A component of path does not name an existing file or path is an empty string.\n");
                break;
            case ENOTDIR:
                printf("Getting queue key failed: A component of the path prefix is not a directory.\n");
                break;
            default:
                printf("Getting queue key failed: %d\n", errno);
                break;
        }
        return -1;
    }

    int msgid = msgget(queueKey, IPC_CREAT|0644);

    if (msgid == -1)
    {
        switch (errno)
        {
            case EACCES:
                perror("Getting queue id failed: A message queue identifier exists for the argument key, but operation permission as specified by the low-order 9 bits of msgflg would not be granted.\n");
                break;
            case EEXIST:
                perror("Getting queue id failed:: A message queue identifier exists for the argument key but ((msgflg & IPC_CREAT) && (msgflg & IPC_EXCL)) is non-zero.\n");
                break;
            case ENOENT:
                perror("Getting queue id failed: A message queue identifier does not exist for the argument key and (msgflg & IPC_CREAT) is 0.\n");
                break;
            case ENOSPC:
                perror("Getting queue id failed: A message queue identifier is to be created but the system-imposed limit on the maximum number of allowed message queue identifiers system-wide would be exceeded.\n");
                break;
            default:
                printf("Getting queue id failed: %d\n", errno);
                break;
        }
    }

    return msgid;
}

/*
*    Prints an error in the message quque system
*/
void PrintMessagingError(int errorNumber)
{
    switch (errorNumber)
    {
        case E2BIG:
            perror("msgrcv error - E2BIG!!\n");
            break;
        case EACCES:
            perror("msgrcv error - EACCES!!\n");
            break;
        case EIDRM:
            perror("msgrcv error - EIDRM!!\n");
            break;
        case EINTR:
            perror("msgrcv error - EINTR!!\n");
            break;
        case EINVAL:
            perror("msgrcv error - EINVAL!!\n");
            break;
        case ENOMSG:
            perror("msgrcv error - ENOMSG!!\n");
            break;
        default:
            break;
    }    
}