#include <iostream>
#include <queue>
#include <string>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

int mode;

ofstream fout("output.txt");

sem_t que; // семафор для очереди пациентов
sem_t surg; // семафор для занятости хирурга
sem_t ther; // семафор для занятости терапевта
sem_t dent; // семафор для занятости стоматолога

queue<string> patients; // очередь пациентов

bool surgBusy = false; // хирург занят?
bool therBusy = false; // терапевт занят?
bool dentBusy = false; // стоматолог занят?

// функция для потока приема у стоматолога
void* dentistAdm(void *param) {
    sem_wait(&dent); // ограничиваем доступ к занятости стоматолога, чтобы он не мог принимать несколько пациентов одновременно
    dentBusy = true; // стоматолог занят
    usleep(3000000); // время приема у стоматолога
    cout << "Dentist has cured one patient\n";
    if (mode == 2) {
        fout << "Dentist has cured one patient\n";
    }
    dentBusy = false; // стоматолог не занят
    sem_post(&dent); // снимаем ограничения
}

// функция для потока приема у хирурга
void* surgeryAdm(void *param) {
    sem_wait(&surg); // ограничиваем доступ к занятости хирурга, чтобы он не мог принимать несколько пациентов одновременно
    surgBusy = true; // хирург занят
    usleep(4000000); // время приема у хирурга
    cout << "Surgery has cured one patient\n";
    if (mode == 2) {
        fout << "Surgery has cured one patient\n";
    }
    surgBusy = false; // хирург не занят
    sem_post(&surg); // снимаем ограничения
}

// функция для потока приема у терапевта
void* therapistAdm(void *param) {
    sem_wait(&ther); // ограничиваем доступ к занятости терапевта, чтобы он не мог принимать несколько пациентов одновременно
    therBusy = true; // терапевт занят
    usleep(2000000); // время приема у терапевта
    cout << "Therapist has cured one patient\n";
    if (mode == 2) {
        fout << "Therapist has cured one patient\n";
    }
    therBusy = false; // терапевт не занят
    sem_post(&ther); // снимаем ограничения
}

// функция для потока приема у дежурного врача
void* admission(void *param) {
    while(!patients.empty()) {
        sem_wait(&que); // оганичиваем доступ к очереди пациентов, что два потока не забрали одного пациента
        string tmp = patients.front();
        patients.pop();
        sem_post(&que); // снимаем ограничения
        if (tmp == "tooth") { // если пациент жалуется на зуб
            while (dentBusy) { // ждем пока стоматолог освободится
                continue;
            }
            pthread_t dentist;
            pthread_create(&dentist, NULL, dentistAdm, NULL); // стоматолог начинает прием
            pthread_join(dentist, nullptr);
        } else if (tmp == "injury") { // если у пациента травма
            while (surgBusy) { // ждем пока хирург освободится
                continue;
            }
            pthread_t surgery;
            pthread_create(&surgery, NULL, surgeryAdm, NULL); // хирург начинает прием
            pthread_join(surgery, nullptr);
        } else { // если проблема другого характера
            while (therBusy) { // ждем пока терапевт освободится
                continue;
            }
            pthread_t therapist;
            pthread_create(&therapist, NULL, therapistAdm, NULL); // терапевт начинает прием
            pthread_join(therapist, nullptr);
        }
    }
}

int main(int argc, char* argv[]) {
    int n;
    if (argc == 1) {
        cout << "Console input mode chosen\n";
        cout << "Input the number of patients:\n";
        cin >> n;
        cout << "Input the issues of each patient:\n";
        for (int i = 0; i < n; ++i) {
            string tmp;
            cin >> tmp;
            patients.push(tmp);
        }
        mode = 1;
    } else if (argc == 2) {
        ifstream fin(argv[1]);
        cout << "File input mode chosen\n";
        fin >> n;
        for (int i = 0; i < n; ++i) {
            string tmp;
            fin >> tmp;
            patients.push(tmp);
        }
        mode = 2;
    }
    pthread_t duty1, duty2;

    // инициализация семафоров
    sem_init(&que, 0, 1);
    sem_init(&dent, 0, 1);
    sem_init(&surg, 0, 1);
    sem_init(&ther, 0, 1);

    // потоки для двух дежурных врачей
    pthread_create(&duty1, NULL, admission, NULL);
    pthread_create(&duty2, NULL, admission, NULL);


    pthread_join(duty1, nullptr);
    pthread_join(duty2, nullptr);

    // уничтожение семафоров
    sem_destroy(&que);
    sem_destroy(&dent);
    sem_destroy(&surg);
    sem_destroy(&ther);
}