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

struct users
{
    int number_of_users;
    int ids[MAX_NUMBER_OF_USERS];
    char names[MAX_NUMBER_OF_USERS][256];
    int current_number_of_users;
};

struct messages_types
{
    int number_of_messages;
    int ids[MAX_NUMBER_OF_MESSAGES];
    char names[MAX_NUMBER_OF_MESSAGES][256];

    int users_ids[MAX_NUMBER_OF_MESSAGES][MAX_NUMBER_OF_USERS];
    char users_names[MAX_NUMBER_OF_MESSAGES][MAX_NUMBER_OF_USERS][256];
    int number_of_users_ids[MAX_NUMBER_OF_MESSAGES];

    int requested_messages_number[MAX_NUMBER_OF_MESSAGES][MAX_NUMBER_OF_USERS];
    int number_of_requested_messages[MAX_NUMBER_OF_MESSAGES];

    char send_message[256];
    int send_message_type;
    int send_message_priority;
};

void Display_program_info();
void Display_end_of_program();
void Display_logged_in_users(struct users *);
void Display_all_messages_types(struct messages_types *);
void Display_subscribers_of_messages_types(struct messages_types *messages_types);

int main(int argc, char *argv[])
{
    Display_program_info();

    int queue_id = msgget(79630, 0644 | IPC_CREAT);

    int shm_id = shmget(IPC_PRIVATE, sizeof(struct users), 0644 | IPC_CREAT);
    struct users *users = (struct users *)shmat(shm_id, NULL, 0);

    users->current_number_of_users = 1;
    users->number_of_users = 0;

    int shm_id2 = shmget(IPC_PRIVATE, sizeof(struct messages_types), 0644 | IPC_CREAT);
    struct messages_types *messages_types = (struct messages_types *)shmat(shm_id2, NULL, 0);

    messages_types->number_of_messages = 0;
    for (int i = 0; i < MAX_NUMBER_OF_MESSAGES; i = i + 1)
    {
        messages_types->number_of_users_ids[i] = 0;
        messages_types->number_of_requested_messages[i] = 0;
        for (int j = 0; j < MAX_NUMBER_OF_USERS; j = j + 1)
        {
            messages_types->requested_messages_number[i][j] = -1;
        }
    }

    int PID_log_in = fork();
    if (PID_log_in == 0)
    {
        while (1)
        {
            msgrcv(queue_id, &login, sizeof(struct msgbuf) - sizeof(long), LOGIN, 0);

            if (users->number_of_users < MAX_NUMBER_OF_USERS)
            {
                printf("----------------------------------\n");
                printf("Dodawanie uzytkownika\n");
                printf("Identyfikator uzytkownika: %d\n", login.id);
                printf("Nazwa uzytkownika: %s\n", login.name);
                printf("----------------------------------\n");

                if (users->number_of_users == 0)
                {
                    users->ids[users->number_of_users] = login.id;
                    strcpy(users->names[users->number_of_users], login.name);
                    printf("Dodano uzytkownika\n");
                    strcpy(login.name, "Zalogowano pomyślnie");
                    if (users->number_of_users > 0)
                    {
                        users->current_number_of_users = users->current_number_of_users + 1;
                    }
                    users->number_of_users = users->number_of_users + 1;
                }
                else
                {
                    int check = 0;
                    for (int i = 0; i < users->number_of_users; i = i + 1)
                    {
                        if ((login.id == users->ids[i]) && (strcmp(login.name, users->names[i]) != 0))
                        {
                            printf("W bazie widnieje juz uzytkownik o takim identyfikatorze\n");
                            printf("Identyfikator: %d\n", users->ids[i]);
                            printf("Nazwa: %s\n", users->names[i]);
                            strcpy(login.name, "Podane ID juz istnieje");
                        }
                        else if ((strcmp(login.name, users->names[i]) == 0) && (login.id != users->ids[i]))
                        {
                            printf("W bazie widnieje juz uzytkownik o takiej nazwie\n");
                            printf("Identyfikator: %d\n", users->ids[i]);
                            printf("Nazwa: %s\n", users->names[i]);
                            strcpy(login.name, "Podana nazwa juz istnieje");
                        }
                        else if ((strcmp(login.name, users->names[i]) == 0) && (login.id == users->ids[i]))
                        {
                            printf("W bazie widnieje juz taki uzytkownik\n");
                            printf("Identyfikator: %d\n", users->ids[i]);
                            printf("Nazwa: %s\n", users->names[i]);
                            strcpy(login.name, "Podany uzytkownik juz istnieje");
                        }
                        else
                        {
                            check = check + 1;
                        }
                    }

                    if (check == users->number_of_users)
                    {
                        users->ids[users->number_of_users] = login.id;
                        strcpy(users->names[users->number_of_users], login.name);
                        printf("Dodano uzytkownika\n");
                        strcpy(login.name, "Zalogowano pomyślnie");
                        if (users->number_of_users > 0)
                        {
                            users->current_number_of_users = users->current_number_of_users + 1;
                        }
                        users->number_of_users = users->number_of_users + 1;
                    }
                }
                login.type = login.id + (ANSWER_LOGIN * MAX_NUMBER_OF_USERS);
                msgsnd(queue_id, &login, sizeof(struct msgbuf) - sizeof(long), 0);
            }
            else
            {
                printf("----------------------------------\n");
                printf("Obecnie serwer jest zapełniony.\n");
                strcpy(login.name, "Obecnie serwer jest zapełniony.");
                login.type = login.id + (ANSWER_LOGIN * MAX_NUMBER_OF_USERS);
                msgsnd(queue_id, &login, sizeof(struct msgbuf) - sizeof(long), 0);
            }
        }
    }
    else
    {
        int PID_registration_new_message_type = fork();
        if (PID_registration_new_message_type == 0)
        {
            while (1)
            {
                msgrcv(queue_id, &message_type, sizeof(struct msgbuf) - sizeof(long), MESSAGE_TYPE, 0);
                printf("----------------------------------\n");
                printf("Rejestracja nowego typu wiadomości uzytkownika\n");
                printf("Identyfikator uzytkownika: %d\n", message_type.id);
                printf("Nazwa uzytkownika: %s\n", message_type.name);

                int new_type_number = message_type.number;
                char new_type_name[256], info[256];
                strcpy(new_type_name, message_type.msg);

                if (new_type_number == 0)
                {
                    new_type_number = 1;
                    int check = 0;
                    if (messages_types->number_of_messages != 0)
                    {
                        for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
                        {
                            for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
                            {
                                if (messages_types->ids[i] == new_type_number)
                                {
                                    new_type_number = new_type_number + 1;
                                    break;
                                }
                                else
                                {
                                    check = check + 1;
                                }
                            }
                            if (check == messages_types->number_of_messages)
                            {
                                break;
                            }
                            else
                            {
                                check = 0;
                            }
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
                    {
                        if ((messages_types->ids[i] == new_type_number) && (strcmp(messages_types->names[i], new_type_name) != 0))
                        {
                            new_type_number = 0;
                            strcpy(info, "Istnieje juz typ o podanym identyfikatorze.");
                            break;
                        }
                        else if ((messages_types->ids[i] != new_type_number) && (strcmp(messages_types->names[i], new_type_name) == 0))
                        {
                            new_type_number = 0;
                            strcpy(info, "Istnieje juz typ o podanej nazwie.");
                            break;
                        }
                        else if ((messages_types->ids[i] == new_type_number) && (strcmp(messages_types->names[i], new_type_name) == 0))
                        {
                            new_type_number = 0;
                            strcpy(info, "Podany typ juz istnieje.");
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }

                if (new_type_number == 0)
                {
                    strcpy(message_type2.msg, info);
                    printf("Niepowodzenie. %s\n", info);
                }
                else
                {
                    messages_types->ids[messages_types->number_of_messages] = new_type_number;
                    strcpy(messages_types->names[messages_types->number_of_messages], new_type_name);

                    printf("Utworzono nowy typ wiadomości: %d %s\n", new_type_number, new_type_name);
                    strcpy(message_type2.msg, "Utworzono nowy typ wiadomości");

                    message_type3.id = message_type.id;
                    message_type3.number = new_type_number;
                    strcpy(message_type3.name, new_type_name);

                    if (users->number_of_users > 0)
                    {
                        for (int i = 0; i < users->number_of_users; i = i + 1)
                        {
                            message_type3.type = users->ids[i] + (NEW_MESSAGE_TYPE * MAX_NUMBER_OF_USERS);
                            msgsnd(queue_id, &message_type3, sizeof(struct msgbuf) - sizeof(long), 0);
                        }
                    }

                    messages_types->number_of_messages = messages_types->number_of_messages + 1;
                }

                message_type2.type = message_type.id + (ANSWER_MESSAGE_TYPE * MAX_NUMBER_OF_USERS);
                msgsnd(queue_id, &message_type2, sizeof(struct msgbuf) - sizeof(long), 0);
            }
        }
        else
        {
            int PID_display_messages_types = fork();
            if (PID_display_messages_types == 0)
            {
                while (1)
                {
                    msgrcv(queue_id, &display_types, sizeof(struct msgbuf) - sizeof(long), DISPLAY_TYPES, 0);
                    printf("----------------------------------\n");
                    printf("Wyświetlenie typow wiadomosci dla uzytkownika\n");
                    printf("Identyfikator uzytkownika: %d\n", display_types.id);
                    printf("Nazwa uzytkownika: %s\n", display_types.name);
                    Display_all_messages_types(messages_types);

                    for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
                    {
                        display_types2.messages_types_ids[i] = messages_types->ids[i];
                        // strcpy(display_types2.messages_types_names[i], messages_types->names[i]);
                    }

                    display_types2.type = display_types.id + (ANSWER_DISPLAY_TYPES * MAX_NUMBER_OF_USERS);
                    display_types2.number = messages_types->number_of_messages;
                    msgsnd(queue_id, &display_types2, sizeof(struct msgbuf) - sizeof(long), 0);
                }
            }
            else
            {
                int PID_registration_subscription = fork();
                if (PID_registration_subscription == 0)
                {
                    while (1)
                    {
                        msgrcv(queue_id, &registration, sizeof(struct msgbuf) - sizeof(long), REGISTRATION, 0);
                        printf("----------------------------------\n");
                        printf("Rejestracja do systemu rozgłaszania uzytkownika\n");
                        printf("Identyfikator uzytkownika: %d\n", registration.id);
                        printf("Nazwa uzytkownika: %s\n", registration.name);

                        int searching = -1;
                        for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
                        {
                            if (messages_types->ids[i] == registration.number)
                            {
                                searching = i;
                                break;
                            }
                        }

                        if ((searching != -1))
                        {
                            int check = 0;
                            for (int i = 0; i < messages_types->number_of_users_ids[searching]; i = i + 1)
                            {
                                if (messages_types->users_ids[searching][i] == registration.id)
                                {
                                    break;
                                }
                                else
                                {
                                    check = check + 1;
                                }
                            }

                            if (check == messages_types->number_of_users_ids[searching])
                            {
                                messages_types->users_ids[searching][messages_types->number_of_users_ids[searching]] = registration.id;
                                strcpy(messages_types->users_names[searching][messages_types->number_of_users_ids[searching]], registration.name);

                                if (strcmp(registration.msg, "s") == 0)
                                {
                                    messages_types->requested_messages_number[searching][messages_types->number_of_requested_messages[searching]] = -1;
                                }
                                else
                                {
                                    messages_types->requested_messages_number[searching][messages_types->number_of_requested_messages[searching]] = registration.number2;
                                }

                                messages_types->number_of_users_ids[searching] = messages_types->number_of_users_ids[searching] + 1;
                                messages_types->number_of_requested_messages[searching] = messages_types->number_of_requested_messages[searching] + 1;

                                char info[256] = "Dodano do bazy subskrybentow";
                                strcpy(registration2.msg, info);
                                printf("%s\n", info);
                            }
                            else
                            {
                                char info[256] = "Uzytkownik subskrybuje juz ten typ wiadomości";
                                strcpy(registration2.msg, info);
                                printf("%s\n", info);
                            }
                        }
                        else
                        {
                            char info[256] = "W bazie nie ma takiego identyfikatora typu wiadomości";
                            strcpy(registration2.msg, info);
                            printf("%s\n", info);
                        }

                        registration2.type = registration.id + (ANSWER_REGISTRATION + MAX_NUMBER_OF_USERS);
                        msgsnd(queue_id, &registration2, sizeof(struct msgbuf) - sizeof(long), 0);
                    }
                }
                else
                {
                    int PID_sending_message = fork();
                    if (PID_sending_message == 0)
                    {
                        while (1)
                        {
                            msgrcv(queue_id, &send_message, sizeof(struct msgbuf) - sizeof(long), SEND_MESSAGE, 0);
                            printf("----------------------------------\n");
                            printf("Rozgłaszanie nowej wiadomości od uzytkownika\n");
                            printf("Identyfikator uzytkownika: %d\n", send_message.id);
                            printf("Nazwa uzytkownika: %s\n", send_message.name);

                            strcpy(messages_types->send_message, send_message.msg);
                            messages_types->send_message_type = send_message.number;
                            messages_types->send_message_priority = send_message.number2;

                            int place;

                            for (int j = 0; j < messages_types->number_of_messages; j = j + 1)
                            {
                                if (messages_types->ids[j] == send_message.number)
                                {
                                    place = j;
                                    break;
                                }
                            }

                            if (users->number_of_users > 0)
                            {
                                for (int i = 0; i < users->number_of_users; i = i + 1)
                                {
                                    for (int j = 0; j < messages_types->number_of_users_ids[place]; j = j + 1)
                                    {
                                        if (messages_types->users_ids[place][j] == users->ids[i])
                                        {
                                            if (messages_types->requested_messages_number[place][j] == 0)
                                            {
                                                continue;
                                            }

                                            if ((messages_types->requested_messages_number[place][j] > 0) && (users->ids[i] != send_message.id))
                                            {
                                                messages_types->requested_messages_number[place][j] = messages_types->requested_messages_number[place][j] - 1;
                                            }

                                            send_message2.type = users->ids[i] + (SENDING * MAX_NUMBER_OF_USERS);
                                            strcpy(send_message2.msg, send_message.msg);
                                            send_message2.id = send_message.id;
                                            send_message2.number2 = send_message.number2;
                                            if (users->ids[i] != send_message.id)
                                            {
                                                msgsnd(queue_id, &send_message2, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        int PID_log_out = fork();
                        if (PID_log_out == 0)
                        {
                            while (1)
                            {
                                msgrcv(queue_id, &logout, sizeof(struct msgbuf) - sizeof(long), LOGOUT, 0);
                                printf("----------------------------------\n");
                                printf("Wylogowywanie uzytkownika\n");
                                printf("Identyfikator uzytkownika: %d\n", logout.id);
                                printf("Nazwa uzytkownika: %s\n", logout.name);

                                users->current_number_of_users = users->current_number_of_users - 1;
                            }
                        }
                        else
                        {
                            int PID_block = fork();
                            if (PID_block == 0)
                            {
                                while (1)
                                {
                                    msgrcv(queue_id, &blocking, sizeof(struct msgbuf) - sizeof(long), BLOCKING, 0);
                                    printf("----------------------------------\n");
                                    printf("Blokowanie pewnego uzytkownika przez uzytkownika\n");
                                    printf("Identyfikator uzytkownika: %d\n", blocking.id);
                                    printf("Nazwa uzytkownika: %s\n", blocking.name);

                                    int check = 0;
                                    for (int i = 0; i < users->number_of_users; i = i + 1)
                                    {
                                        if (users->ids[i] == blocking.number)
                                        {
                                            blocking2.number = 1;
                                            break;
                                        }
                                        else
                                        {
                                            check = check + 1;
                                        }
                                    }
                                    if (check == users->number_of_users)
                                    {
                                        blocking2.number = 0;
                                    }

                                    blocking2.type = blocking.id + (ANSWER_BLOCKING * MAX_NUMBER_OF_USERS);
                                    blocking2.id = blocking.id;
                                    strcpy(blocking2.name, blocking.name);
                                    msgsnd(queue_id, &blocking2, sizeof(struct msgbuf) - sizeof(long), 0);
                                }
                            }
                            else
                            {
                                while (1)
                                {
                                    if (users->current_number_of_users == 0)
                                    {
                                        break;
                                    }
                                    sleep(SERVER_STANDBY_TIME);
                                }
                                Display_logged_in_users(users);
                                Display_all_messages_types(messages_types);
                                Display_subscribers_of_messages_types(messages_types);
                                Display_end_of_program();
                                kill(PID_log_in, 9);
                                kill(PID_registration_new_message_type, 9);
                                kill(PID_display_messages_types, 9);
                                kill(PID_registration_subscription, 9);
                                kill(PID_sending_message, 9);
                                kill(PID_log_out, 9);
                                kill(PID_block, 9);
                                msgctl(queue_id, IPC_RMID, NULL);
                                shmdt(users);
                                shmdt(messages_types);
                                shmctl(shm_id, IPC_RMID, NULL);
                                shmctl(shm_id2, IPC_RMID, NULL);
                                exit(0);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void Display_program_info()
{
    printf("----------------------------------\n");
    printf("IPC PUBLISH-SUBSCRIBE - Serwer\n");
    printf("Created by Filip Ciesielski\n");
    printf("----------------------------------\n");
    printf("Serwer rozpoczyna pracę\n");
}

void Display_end_of_program()
{
    printf("----------------------------------\n");
    printf("Wszyscy uzytkownicy wylogowali się z systemu\n");
    printf("Koniec pracy serwera\n");
    printf("----------------------------------\n");
}

void Display_logged_in_users(struct users *users)
{
    printf("----------------------------------\n");
    printf("Zalogowani uzytkownicy:\n");
    for (int i = 0; i < users->number_of_users; i = i + 1)
    {
        printf("%d. Identyfikator: %d Nazwa: %s\n", i + 1, users->ids[i], users->names[i]);
    }
}

void Display_all_messages_types(struct messages_types *messages_types)
{
    printf("----------------------------------\n");
    printf("Typy wiadomości:\n");
    for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
    {
        printf("%d. Identyfikator: %d Nazwa: %s\n", i + 1, messages_types->ids[i], messages_types->names[i]);
    }
}

void Display_subscribers_of_messages_types(struct messages_types *messages_types)
{
    printf("----------------------------------\n");
    printf("Subskrybenci danych typow:\n");
    for (int i = 0; i < messages_types->number_of_messages; i = i + 1)
    {
        printf("%d. Identyfikator typu: %d Nazwa typu: %s\n", i + 1, messages_types->ids[i], messages_types->names[i]);
        for (int j = 0; j < messages_types->number_of_users_ids[i]; j = j + 1)
        {
            printf("\t%d. Identyfikator subskrybenta: %d Nazwa subskrybenta: %s\n", j + 1, messages_types->users_ids[i][j], messages_types->users_names[i][j]);
        }
    }
}
