#include <sys/msg.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NUMBER_OF_USERS 5
#define MAX_LOGIN_ATTEMPTS 3
#define MAX_NUMBER_OF_MESSAGES 100
#define SERVER_STANDBY_TIME 10

#define LOGIN 1
#define ANSWER_LOGIN 9

#define MESSAGE_TYPE 2
#define ANSWER_MESSAGE_TYPE 10

#define DISPLAY_TYPES 3
#define ANSWER_DISPLAY_TYPES 11

#define REGISTRATION 4
#define ANSWER_REGISTRATION 12

#define BLOCKING 5
#define ANSWER_BLOCKING 13

#define NEW_MESSAGE_TYPE 14
#define SEND_MESSAGE 6
#define LOGOUT 7
#define SENDING 8

struct msgbuf
{
    long type;
    int id;
    char name[256];
    int number, number2;
    char msg[256];
    int messages_types_ids[MAX_NUMBER_OF_MESSAGES];
    // char messages_types_names[MAX_NUMBER_OF_MESSAGES][256];
} login, login2, message_type, message_type2, message_type3, display_types, display_types2, registration, registration2, send_message, send_message2, logout, blocking, blocking2;

struct user
{
    int id;
    char name[256];
    int check;
    int subscription[MAX_NUMBER_OF_MESSAGES];
    int number_of_subscriptions;
    int blocked_users[MAX_NUMBER_OF_USERS - 1];
    int number_of_blocked_users;
};

struct messages
{
    char message[MAX_NUMBER_OF_MESSAGES][256];
    int message_priority[MAX_NUMBER_OF_MESSAGES];
    int number_of_messages;
};

void Display_program_info();
void Display_menu();

int main(int argc, char *argv[])
{
    Display_program_info();

    int attempts = 0, blocked, PID_info_asynchronous_message;

    int queue_id = msgget(79630, 0644 | IPC_CREAT);

    int shm_id = shmget(IPC_PRIVATE, sizeof(struct user), 0644 | IPC_CREAT);
    struct user *user = (struct user *)shmat(shm_id, NULL, 0);

    user->check = 0;
    user->number_of_subscriptions = 0;
    user->number_of_blocked_users = 0;

    int shm_id2 = shmget(IPC_PRIVATE, sizeof(struct messages), 0644 | IPC_CREAT);
    struct messages *messages = (struct messages *)shmat(shm_id2, NULL, 0);

    messages->number_of_messages = 0;

    do
    {
        printf("Logowanie do systemu rozgłaszania\n");
        do
        {
            printf("Wprowadź identyfikator: ");
            scanf("%d", &user->id);
        } while (user->id <= 0);
        printf("Wprowadź nazwę: ");
        scanf("%s", user->name);
        printf("----------------------------------\n");

        login.type = LOGIN;
        strcpy(login.name, user->name);
        login.id = user->id;

        msgsnd(queue_id, &login, sizeof(struct msgbuf) - sizeof(long), 0);

        printf("Informacja zwrotna od serwera:\n");
        msgrcv(queue_id, &login2, sizeof(struct msgbuf) - sizeof(long), user->id + (ANSWER_LOGIN * MAX_NUMBER_OF_USERS), 0);
        printf("%s\n", login2.name);
        printf("----------------------------------\n");

        attempts = attempts + 1;

    } while (attempts < MAX_LOGIN_ATTEMPTS && (strcmp(login2.name, "Obecnie serwer jest zapełniony.") != 0) && (strcmp(login2.name, "Zalogowano pomyślnie") != 0));

    if ((attempts >= MAX_LOGIN_ATTEMPTS) && (strcmp(login2.name, "Zalogowano pomyślnie") != 0))
    {
        printf("Przekroczono dopuszczalną ilość prob logowania\n");
        printf("----------------------------------\n");
        shmdt(user);
        shmctl(shm_id, IPC_RMID, NULL);
        shmdt(messages);
        shmctl(shm_id2, IPC_RMID, NULL);
        return 0;
    }
    else if (strcmp(login2.name, "Obecnie serwer jest zapełniony.") == 0)
    {
        shmdt(user);
        shmctl(shm_id, IPC_RMID, NULL);
        shmdt(messages);
        shmctl(shm_id2, IPC_RMID, NULL);
        return 0;
    }
    else if (strcmp(login2.name, "Zalogowano pomyślnie") == 0)
    {
        int PID_info_new_message_type = fork();
        if (PID_info_new_message_type == 0)
        {
            while (1)
            {
                msgrcv(queue_id, &message_type3, sizeof(struct msgbuf) - sizeof(long), user->id + (NEW_MESSAGE_TYPE * MAX_NUMBER_OF_USERS), 0);
                if (message_type3.id != user->id)
                {
                    printf("\n----------------------------------\n");
                    printf("Utworzono nowy typ wiadomości\n");
                    printf("Identyfikator: %d\n", message_type3.number);
                    printf("Nazwa: %s\n", message_type3.name);
                    printf("----------------------------------\n\n");
                }
                else
                {
                    continue;
                }
            }
        }
        else
        {
            Display_menu();
            do
            {
                int task;
                printf("Wybierz działanie: ");
                scanf("%d", &task);
                printf("----------------------------------\n");

                switch (task)
                {
                case 0:
                    printf("Informacja zwrotna od serwera:\n");
                    printf("Wylogowano pomyślnie\n");
                    printf("----------------------------------\n");
                    logout.id = user->id;
                    strcpy(logout.name, user->name);
                    logout.type = LOGOUT;
                    msgsnd(queue_id, &logout, sizeof(struct msgbuf) - sizeof(long), 0);

                    kill(PID_info_new_message_type, 9);
                    if (user->check == 1)
                    {
                        kill(PID_info_asynchronous_message, 9);
                    }
                    shmdt(user);
                    shmdt(messages);
                    shmctl(shm_id, IPC_RMID, NULL);
                    shmctl(shm_id2, IPC_RMID, NULL);
                    return 0;
                case 1:
                    Display_menu();
                    break;
                case 2:
                    printf("Wyświetlenie dostępnych typow wiadomości do subskrypcji\n");
                    display_types.type = DISPLAY_TYPES;
                    display_types.id = user->id;
                    strcpy(display_types.name, user->name);
                    msgsnd(queue_id, &display_types, sizeof(struct msgbuf) - sizeof(long), 0);

                    msgrcv(queue_id, &display_types2, sizeof(struct msgbuf) - sizeof(long), user->id + (ANSWER_DISPLAY_TYPES * MAX_NUMBER_OF_USERS), 0);

                    for (int i = 0; i < display_types2.number; i = i + 1)
                    {
                        printf("%d. Identyfikator: %d\n", i + 1, display_types2.messages_types_ids[i]);
                        // printf("Nazwa: %s\n", display_types2.messages_types_names[i]);
                    }

                    printf("----------------------------------\n");
                    break;
                case 3:
                    printf("Rejestracja do systemu rozgłaszania (subskrypcja odpowiedniego typu)\n");

                    registration.type = REGISTRATION;
                    registration.id = user->id;
                    strcpy(registration.name, user->name);

                    printf("Wprowadź identyfikator typu wiadomości do subskrypcji: ");
                    scanf("%d", &registration.number);

                    printf("Wybierz sposob odbioru wiadomości\n");
                    do
                    {
                        printf("Subskrypcja stała czy przejściowa? [s]/[p]: ");
                        scanf("%s", registration.msg);
                    } while ((strcmp(registration.msg, "s") != 0) && (strcmp(registration.msg, "p") != 0));

                    if (strcmp(registration.msg, "p") == 0)
                    {
                        printf("Wprowadz liczbę wiadomości do odbioru: ");
                        scanf("%d", &registration.number2);
                    }

                    msgsnd(queue_id, &registration, sizeof(struct msgbuf) - sizeof(long), 0);

                    printf("----------------------------------\n");
                    printf("Informacja zwrotna od serwera:\n");

                    msgrcv(queue_id, &registration2, sizeof(struct msgbuf) - sizeof(long), user->id + (ANSWER_REGISTRATION + MAX_NUMBER_OF_USERS), 0);
                    printf("%s\n", registration2.msg);

                    if (strcmp(registration2.msg, "Dodano do bazy subskrybentow") == 0)
                    {
                        user->subscription[user->number_of_subscriptions] = registration.number;
                        user->number_of_subscriptions = user->number_of_subscriptions + 1;
                    }

                    printf("----------------------------------\n");
                    break;
                case 4:
                    printf("Rejestracja typu wiadomości do systemu rozgłaszania\n");

                    printf("Wybierz sposob rejestracji typu wiadomości\n");
                    char msg_type[2];
                    do
                    {
                        printf("rejestracja samodzielna czy automatyczna systemowa (unikalna)? [s]/[a]: ");
                        scanf("%s", msg_type);
                    } while ((strcmp(msg_type, "s") != 0) && (strcmp(msg_type, "a") != 0));

                    char new_type[256];
                    strcpy(new_type, "systemowy typ");
                    int new_type_number = 0;

                    if (strcmp(msg_type, "s") == 0)
                    {
                        do
                        {
                            printf("Wprowadź identyfikator tego typu: ");
                            scanf("%d", &new_type_number);
                        } while (new_type_number == 0);
                        do
                        {
                            printf("Wprowadź nazwę nowego typu wiadomości: ");
                            scanf("%s", new_type);
                        } while (strcmp(new_type, "systemowy typ") == 0);
                    }

                    message_type.type = MESSAGE_TYPE;
                    message_type.id = user->id;
                    strcpy(message_type.name, user->name);
                    strcpy(message_type.msg, new_type);
                    message_type.number = new_type_number;

                    msgsnd(queue_id, &message_type, sizeof(struct msgbuf) - sizeof(long), 0);

                    printf("----------------------------------\n");
                    printf("Informacja zwrotna od serwera:\n");

                    msgrcv(queue_id, &message_type2, sizeof(struct msgbuf) - sizeof(long), user->id + (ANSWER_MESSAGE_TYPE * MAX_NUMBER_OF_USERS), 0);
                    printf("%s\n", message_type2.msg);
                    printf("----------------------------------\n");
                    break;
                case 5:
                    printf("Rozgłoszenie nowej wiadomości\n");
                    printf("Wprowadź wiadomość do rozgłoszenia: ");
                    scanf("%s", send_message.msg);
                    printf("Wprowadź typ wysyłanej wiadomości: ");
                    scanf("%d", &send_message.number);
                    do
                    {
                        printf("Wprowadź priorytet wysyłanej wiaodmości (>0): ");
                        scanf("%d", &send_message.number2);
                    } while (send_message.number2 <= 0);
                    printf("----------------------------------\n");

                    send_message.id = user->id;
                    strcpy(send_message.name, user->name);
                    send_message.type = SEND_MESSAGE;

                    msgsnd(queue_id, &send_message, sizeof(struct msgbuf) - sizeof(long), 0);
                    break;
                case 6:
                    printf("Oczekiwanie na wiadomość w sposob synchroniczny\n");
                    printf("----------------------------------\n");

                    msgrcv(queue_id, &send_message2, sizeof(struct msgbuf) - sizeof(long), user->id + (SENDING * MAX_NUMBER_OF_USERS), 0);

                    int check = 0;
                    for (int i = 0; i < user->number_of_blocked_users; i = i + 1)
                    {
                        if (user->blocked_users[i] == send_message2.id)
                        {
                            check = 1;
                            strcpy(messages->message[messages->number_of_messages], "");
                            messages->message_priority[messages->number_of_messages] = 0;
                            messages->number_of_messages = messages->number_of_messages + 1;
                            break;
                        }
                    }

                    if (check == 0)
                    {
                        if (send_message2.id != user->id)
                        {
                            strcpy(messages->message[messages->number_of_messages], send_message2.msg);
                            messages->message_priority[messages->number_of_messages] = send_message2.number2;
                            messages->number_of_messages = messages->number_of_messages + 1;
                        }
                    }

                    while (1)
                    {
                        int check2 = 0;
                        int check = msgrcv(queue_id, &send_message2, sizeof(struct msgbuf) - sizeof(long), user->id + (SENDING * MAX_NUMBER_OF_USERS), IPC_NOWAIT);

                        if (check != -1)
                        {
                            for (int i = 0; i < user->number_of_blocked_users; i = i + 1)
                            {
                                if (user->blocked_users[i] == send_message2.id)
                                {
                                    check2 = 1;
                                    strcpy(messages->message[messages->number_of_messages], "");
                                    messages->message_priority[messages->number_of_messages] = 0;
                                    messages->number_of_messages = messages->number_of_messages + 1;
                                    break;
                                }
                            }

                            if (check2 == 0)
                            {
                                if (send_message2.id != user->id)
                                {
                                    strcpy(messages->message[messages->number_of_messages], send_message2.msg);
                                    messages->message_priority[messages->number_of_messages] = send_message2.number2;
                                    messages->number_of_messages = messages->number_of_messages + 1;
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    printf("\n----------------------------------\n");
                    printf("Odebrano wiadomość w sposob synchroniczny:\n");

                    for (int j = 0; j < messages->number_of_messages; j = j + 1)
                    {
                        for (int i = 1; i < messages->number_of_messages; i = i + 1)
                        {
                            if (messages->message_priority[i - 1] < messages->message_priority[i])
                            {
                                char swap[256];
                                int swapper;

                                strcpy(swap, messages->message[i]);
                                swapper = messages->message_priority[i];

                                strcpy(messages->message[i], messages->message[i - 1]);
                                messages->message_priority[i] = messages->message_priority[i - 1];

                                strcpy(messages->message[i - 1], swap);
                                messages->message_priority[i - 1] = swapper;
                            }
                        }
                    }

                    for (int i = 0; i < messages->number_of_messages; i = i + 1)
                    {
                        if (messages->message_priority[i] != 0)
                        {
                            printf("%d. %s\n", i + 1, messages->message[i]);
                            printf("----------------------------------\n");
                        }
                        else if (messages->message_priority[i] == 0 && messages->number_of_messages == 1)
                        {
                            printf("Informacja zwrotna: odbior wiadomości od tego uzytkownika został zablokowany\n");
                            printf("----------------------------------\n");
                        }
                        else
                        {
                            continue;
                        }
                    }

                    printf("\n");
                    messages->number_of_messages = 0;

                    break;
                case 7:

                    if (user->check == 0)
                    {
                        user->check = 1;
                        printf("Oczekiwanie na wiadomość w sposob asynchroniczny\n");
                        printf("----------------------------------\n");
                        PID_info_asynchronous_message = fork();
                        if (PID_info_asynchronous_message == 0)
                        {
                            msgrcv(queue_id, &send_message2, sizeof(struct msgbuf) - sizeof(long), user->id + (SENDING * MAX_NUMBER_OF_USERS), 0);

                            int check = 0;
                            for (int i = 0; i < user->number_of_blocked_users; i = i + 1)
                            {
                                if (user->blocked_users[i] == send_message2.id)
                                {
                                    check = 1;
                                    strcpy(messages->message[messages->number_of_messages], "");
                                    messages->message_priority[messages->number_of_messages] = 0;
                                    messages->number_of_messages = messages->number_of_messages + 1;
                                    break;
                                }
                            }

                            if (check == 0)
                            {
                                if (send_message2.id != user->id)
                                {
                                    strcpy(messages->message[messages->number_of_messages], send_message2.msg);
                                    messages->message_priority[messages->number_of_messages] = send_message2.number2;
                                    messages->number_of_messages = messages->number_of_messages + 1;
                                }
                            }

                            while (1)
                            {
                                int check2 = 0;
                                int check = msgrcv(queue_id, &send_message2, sizeof(struct msgbuf) - sizeof(long), user->id + (SENDING * MAX_NUMBER_OF_USERS), IPC_NOWAIT);

                                if (check != -1)
                                {
                                    for (int i = 0; i < user->number_of_blocked_users; i = i + 1)
                                    {
                                        if (user->blocked_users[i] == send_message2.id)
                                        {
                                            check2 = 1;
                                            strcpy(messages->message[messages->number_of_messages], "");
                                            messages->message_priority[messages->number_of_messages] = 0;
                                            messages->number_of_messages = messages->number_of_messages + 1;
                                            break;
                                        }
                                    }

                                    if (check2 == 0)
                                    {
                                        if (send_message2.id != user->id)
                                        {
                                            strcpy(messages->message[messages->number_of_messages], send_message2.msg);
                                            messages->message_priority[messages->number_of_messages] = send_message2.number2;
                                            messages->number_of_messages = messages->number_of_messages + 1;
                                        }
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            printf("\n----------------------------------\n");
                            printf("Odebrano wiadomość w sposob asynchroniczny:\n");

                            for (int j = 0; j < messages->number_of_messages; j = j + 1)
                            {
                                for (int i = 1; i < messages->number_of_messages; i = i + 1)
                                {
                                    if (messages->message_priority[i - 1] < messages->message_priority[i])
                                    {
                                        char swap[256];
                                        int swapper;

                                        strcpy(swap, messages->message[i]);
                                        swapper = messages->message_priority[i];

                                        strcpy(messages->message[i], messages->message[i - 1]);
                                        messages->message_priority[i] = messages->message_priority[i - 1];

                                        strcpy(messages->message[i - 1], swap);
                                        messages->message_priority[i - 1] = swapper;
                                    }
                                }
                            }

                            for (int i = 0; i < messages->number_of_messages; i = i + 1)
                            {
                                if (messages->message_priority[i] != 0)
                                {
                                    printf("%d. %s\n", i + 1, messages->message[i]);
                                    printf("----------------------------------\n");
                                }
                                else if (messages->message_priority[i] == 0 && messages->number_of_messages == 1)
                                {
                                    printf("Informacja zwrotna: odbior wiadomości od tego uzytkownika został zablokowany\n");
                                    printf("----------------------------------\n");
                                }
                                else
                                {
                                    continue;
                                }
                            }

                            printf("\n");
                            messages->number_of_messages = 0;

                            user->check = 0;
                            exit(0);
                        }
                        else
                        {
                            usleep(100000);
                            break;
                        }
                    }
                    else
                    {
                        printf("Odbior asynchroniczny został juz zainicjowany\n");
                        printf("----------------------------------\n");
                        break;
                    }
                case 8:
                    printf("Blokowanie innego uzytkownika\n");
                    printf("Wprowadź identyfikator uzytkownika, ktorego chcesz zablokować: ");
                    scanf("%d", &blocked);
                    printf("----------------------------------\n");
                    printf("Informacja zwrotna od serwera:\n");

                    blocking.id = user->id;
                    strcpy(blocking.name, user->name);
                    blocking.number = blocked;
                    blocking.type = BLOCKING;
                    msgsnd(queue_id, &blocking, sizeof(struct msgbuf) - sizeof(long), 0);

                    msgrcv(queue_id, &blocking2, sizeof(struct msgbuf) - sizeof(long), user->id + (ANSWER_BLOCKING * MAX_NUMBER_OF_USERS), 0);

                    if (blocking2.number == 1)
                    {
                        if (blocked == user->id)
                        {
                            printf("Nie mozesz zablokować samego siebie\n");
                        }
                        else
                        {
                            int check = 0;
                            for (int i = 0; i < user->number_of_blocked_users; i++)
                            {
                                if (user->blocked_users[i] == blocked)
                                {
                                    printf("Podany uzytkownik został juz zablokowany\n");
                                    break;
                                }
                                else
                                {
                                    check = check + 1;
                                }
                            }
                            if (check == user->number_of_blocked_users)
                            {
                                user->blocked_users[user->number_of_blocked_users] = blocked;
                                user->number_of_blocked_users = user->number_of_blocked_users + 1;
                                printf("Uzytkownik został zablokowany\n");
                            }
                        }
                    }
                    else
                    {
                        printf("Brak uzytkownika o podanym identyfikatorze\n");
                    }
                    printf("----------------------------------\n");
                    break;
                default:
                    printf("Brak działania o podanym numerze\n");
                    printf("----------------------------------\n");
                    break;
                }
            } while (1);
        }
    }
    return 0;
}

void Display_program_info()
{
    printf("----------------------------------\n");
    printf("IPC PUBLISH-SUBSCRIBE - Klient\n");
    printf("Created by Filip Ciesielski\n");
    printf("----------------------------------\n");
}

void Display_menu()
{
    printf("Dostępne działania do wyboru:\n");
    printf("[0] Wylogowanie się z systemu\n");
    printf("[1] Wyświetlenie dostępnych działań\n");
    printf("[2] Wyświetlenie dostępnych typow wiadomości do subskrypcji\n");
    printf("[3] Rejestracja do systemu rozgłaszania (subskrypcja odpowiedniego typu)\n");
    printf("[4] Rejestracja typu wiadomości\n");
    printf("[5] Rozgłoszenie nowej wiadomości\n");
    printf("[6] Odbior wiadomości w sposob synchroniczny\n");
    printf("[7] Odbior wiadomości w sposob asynchroniczny\n");
    printf("[8] Blokowanie innego uzytkownika\n");
    printf("----------------------------------\n");
}
