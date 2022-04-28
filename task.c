#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

int flag = 1;

static void sig_start(int signo) //Функция - обработчик сигнала
{
    flag = 0; //Прекратить ожидание, если оно имеет место "по вине" flag
}

void main()
{
    int status, pid1, pid2, pid, i;
    int pipe1[2], pipe2[2], sizeofbuf = 50;
    char buf[sizeofbuf], *info;

    //Создание каналов
    printf("Родитель: Создание К1\n");
    if (pipe(pipe1) == -1)
    {
        printf("Родитель: Ошибка создания К1");
        exit(1);
    }
    printf("Родитель: Создание К2\n");
    if (pipe(pipe2) == -1)
    {
        printf("Родитель: Ошибка создания К2");
        exit(1);
    }

    printf("Родитель: Создание потомка P1\n");
    pid = fork();
    if (pid == -1)
    {
        printf("Родитель: Ошибка создания P1");
        exit(1);
    }
    if (pid == 0)
    {
        signal(SIGUSR1, sig_start); //Перехватывание сигнала SIGUSR1 функцией sig_start
        printf("Потомок 1: Создание потомка P2\n");
        pid = fork();
        if (pid == -1)
        {
            printf("Потомок 1: Ошибка создания P2");
            exit(1);
        }
        if (pid == 0)
        {
            signal(SIGUSR1, sig_start); //Перехватывание сигнала SIGUSR1 функцией sig_start

            printf("Потомок 2: Ожидание сигнала\n");

            //Потомок P2 приостанавливает свое выполнение до получения сигнала SIGUSR1
            while (flag)
                sleep(1);

            //Формируем информацию для канала
            ((int *)buf)[0] = getpid();
            sprintf(buf + 4, "Информация от потомка P2.");

            //Записываем в открытый предком К2 информацию от P2
            printf("Потомок 2: Отправка информации в К2\n");
            write(pipe2[1], buf, sizeofbuf);

            pid1 = getppid();

            printf("Потомок 2: Отправка сигнала потомку P1\n");
            kill(pid1, SIGUSR1);

            printf("Потомок 2: Завершен\n");
            exit(0);
        }
        else
        {
            pid2 = pid;

            //Формируем информацию для канала
            ((int *)buf)[0] = getpid();
            sprintf(buf + 4, "Информация от потомка Р1");

            //Записываем в К1 информацию от P1
            printf("Потомок 1: Отправка информации в К1\n");
            write(pipe1[1], buf, sizeofbuf);

            printf("Потомок 1: Отправка сигнала потомку Р2\n");
            kill(pid2, SIGUSR1);

            //Потомок P1 приостанавливает свое выполнение до получения сигнала SIGUSR1
            printf("Потомок 1: Ожидание сигнала\n");
            while (flag)
                sleep(1);

            //Потомок P1 считывает информацию из К2
            read(pipe2[0], &buf, sizeofbuf);

            //Потомок P1 записывает эту информацию в К1
            printf("Потомок 1: Запись информации из К2 в К1\n");
            write(pipe1[1], &buf, sizeofbuf);

            //Формируем информацию для К1
            ((int *)buf)[0] = getpid();
            sprintf(buf + 4, "Информация от потомка Р1");

            //Записываем в К1 новую информацию от P1
            printf("Потомок 1: Отправка информации в К1\n");
            write(pipe1[1], buf, sizeofbuf);

            //Ожидаем завершения работы потомка P1
            wait(&status);
            printf("Потомок 1: Завершен\n");
            exit(0);
        }
    }
    else
    {
        //Сохраняем id потомка P1
        pid1 = pid;
        wait(&status);
        printf("Родитель: Получение информации\n");

        for (i = 0; i < 3; i++)
        {
            //Читаем из канала информацию подготовленную потомками
            read(pipe1[0], &buf, sizeofbuf);

            //Получаем номер процесса записавшего эти данные в канал
            pid = *((int *)buf);
            info = buf + sizeof(int); // Получаем послание
            printf("Родитель: Информация от процесса %d :\"%s\"\n", pid, info);
        }
        //Ожидааем завершения работы потомка
        wait(&status);
        printf("Родитель: Завершен\n");
    }
}
