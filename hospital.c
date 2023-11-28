#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

#define HIGH_PRIORITY_LED_PIN 0 
#define NORMAL_PRIORITY_LED_PIN 2 
#define HIGH_PRIORITY_BUTTON_PIN 23 
#define NORMAL_PRIORITY_BUTTON_PIN 25 
#define SERVE_BUTTON_PIN 29 

int highPriorityCountdown = 4;
int highPriorityQueue = 0;
int normalPriorityQueue = 0;
int servedHighPriorityCount = 0;
int servedNormalPriorityCount = 0;

int systemInitialized = 0; 

void setup() {
    wiringPiSetup();
    pinMode(HIGH_PRIORITY_LED_PIN, OUTPUT);
    pinMode(NORMAL_PRIORITY_LED_PIN, OUTPUT);
    pinMode(HIGH_PRIORITY_BUTTON_PIN, INPUT);
    pinMode(NORMAL_PRIORITY_BUTTON_PIN, INPUT);
    pinMode(SERVE_BUTTON_PIN, INPUT);

    pullUpDnControl(HIGH_PRIORITY_BUTTON_PIN, PUD_UP);
    pullUpDnControl(NORMAL_PRIORITY_BUTTON_PIN, PUD_UP);
    pullUpDnControl(SERVE_BUTTON_PIN, PUD_UP);
    
    systemInitialized = 1; 
    
    delay(1000); 
}

void initializeFromPatientsQueueFile() {
    FILE *file = fopen("/root/patients.txt", "r");
    if (file == NULL) {
        printf("Arquivo inicial nao existe. Criando novo e inicializando com valores em zero\n");
        file = fopen("/root/patients.txt", "w");
        if (file == NULL) {
            printf("Erro ao criar o arquivo /root/patients.txt\n");
            return;
        }
        fprintf(file, "%d %d %d %d", 0, 0, 0, 0);
        fclose(file);
        file = fopen("/root/patients.txt", "r");
    }
    
    if (file != NULL) {
        fscanf(file, "%d %d %d %d", &highPriorityQueue, &normalPriorityQueue, &servedHighPriorityCount, &servedNormalPriorityCount);
        printf("Arquivo inicial existe. Obtendo valores dele\n");
        fclose(file);
    } else {
        printf("Erro ao abrir o arquivo /root/patients.txt para leitura\n");
    }
}

void saveCurrentState() {
    FILE *file = fopen("/root/patients.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo patients.txt\n");
        return;
    }
    fprintf(file, "%d %d %d %d", highPriorityQueue, normalPriorityQueue, servedHighPriorityCount, servedNormalPriorityCount);
    fclose(file);
    fflush(file);
}

void servePatient(int patientType) {
    if (patientType == 1) {
        printf("Atendendo Paciente de Prioridade ALTA - Número: %d\n", servedHighPriorityCount);
        servedHighPriorityCount++;
    } else {
        printf("Atendendo Paciente de Prioridade NORMAL - Número: %d\n", servedNormalPriorityCount);
        servedNormalPriorityCount++;
    }

}

void checkButtons() {
    if (!systemInitialized) {
        return;
    }
    static int lastHighPriorityButtonState = HIGH;
    static int lastNormalPriorityButtonState = HIGH;
    static int lastServeButtonState = HIGH;

    int highPriorityButtonState = digitalRead(HIGH_PRIORITY_BUTTON_PIN);
    int normalPriorityButtonState = digitalRead(NORMAL_PRIORITY_BUTTON_PIN);
    int serveButtonState = digitalRead(SERVE_BUTTON_PIN);

    if (highPriorityButtonState == LOW && lastHighPriorityButtonState == HIGH) {
        highPriorityQueue++;
        printf("Chegou Paciente de prioridade: ALTA\n");
        digitalWrite(NORMAL_PRIORITY_LED_PIN, LOW);
    	digitalWrite(HIGH_PRIORITY_LED_PIN, HIGH);
        delay(100);

    }

    if (normalPriorityButtonState == LOW && lastNormalPriorityButtonState == HIGH) {
        normalPriorityQueue++;
        printf("Chegou Paciente de prioridade: NORMAL \n");
        digitalWrite(HIGH_PRIORITY_LED_PIN, LOW);
    	digitalWrite(NORMAL_PRIORITY_LED_PIN, HIGH);
        delay(100);
    }
    
    if (serveButtonState == LOW && lastServeButtonState == HIGH) {
	checkQueues();
	delay(100);
    }
    

    lastHighPriorityButtonState = highPriorityButtonState;
    lastNormalPriorityButtonState = normalPriorityButtonState;
    lastServeButtonState = serveButtonState;
}

void checkQueues() {
    if (((highPriorityCountdown > 0 && highPriorityCountdown <= 4) && highPriorityQueue > 0) || (highPriorityQueue > 0 && normalPriorityQueue == 0)) {
        servePatient(1); 
        highPriorityQueue--;
        highPriorityCountdown--;
    }
    else if (highPriorityCountdown == 0 && normalPriorityQueue > 0) {
        servePatient(0); 
        normalPriorityQueue--;
        highPriorityCountdown = 4;
    }
    else if (highPriorityQueue == 0 && normalPriorityQueue > 0) {
        servePatient(0);
        normalPriorityQueue--;
    }
}

int main() {
    setup();
    initializeFromPatientsQueueFile();
    while (1) {
        checkButtons();
        delay(100);
        saveCurrentState();
    }
    return 0;
}

