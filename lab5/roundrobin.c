#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int remaining;
    int first_response;
    int waiting_time;
    int turnaround_time;
    bool started;
} Process;

int main() {
    FILE *input = fopen("input.txt", "r");
    if (input == NULL) {
        return 1;
    }

    int n, quantum;
    fscanf(input, "%d %d", &n, &quantum);

    Process processes[n];
    for (int i = 0; i < n; i++) {
        fscanf(input, "%d %d %d", &processes[i].id, &processes[i].arrival, &processes[i].burst);
        processes[i].remaining = processes[i].burst;
        processes[i].first_response = -1;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].started = false;
    }

    fclose(input);

    int time = 0, completed = 0;
    float total_waiting_time = 0, total_turnaround_time = 0;
    int queue[100];
    int front = 0, rear = 0;

    bool in_queue[n];
    for (int i = 0; i < n; i++) {
        in_queue[i] = false;
    }

    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !in_queue[i] && processes[i].remaining > 0) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        if (front == rear) {
            time++;
            continue;
        }

        int current = queue[front++];
        if (processes[current].first_response == -1) {
            processes[current].first_response = time;
        }

        if (processes[current].remaining > quantum) {
            time += quantum;
            processes[current].remaining -= quantum;
        } else {
            time += processes[current].remaining;
            processes[current].remaining = 0;
            completed++;
            processes[current].turnaround_time = time - processes[current].arrival;
            processes[current].waiting_time = processes[current].turnaround_time - processes[current].burst;
            total_waiting_time += processes[current].waiting_time;
            total_turnaround_time += processes[current].turnaround_time;
        }

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !in_queue[i] && processes[i].remaining > 0) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        if (processes[current].remaining > 0) {
            queue[rear++] = current;
        }
    }

    FILE *output = fopen("output.txt", "w");
    if (output == NULL) {
        return 1;
    }

    for (int i = 0; i < n; i++) {
        fprintf(output, "%d %d %d %d\n",
                processes[i].id,
                processes[i].first_response,
                processes[i].waiting_time,
                processes[i].turnaround_time);
    }

    fprintf(output, "%.2f\n", total_waiting_time / n);
    fprintf(output, "%.2f\n", total_turnaround_time / n);

    fclose(output);

    return 0;
}
