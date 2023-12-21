#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE    1024
#define BYTES_PER_LINE 10
#define INPUT_SIZE     100
#define HEX_BASE       16
#define BASE_10        10

void print_hex(unsigned char* buffer, size_t bytes_read) {
  for (size_t i = 0; i < bytes_read; i++) {
    if (i % BYTES_PER_LINE == 0) {
      printf("\n");
    }
    printf("%.2X ", buffer[i]);
  }
}

void print_command(unsigned char* buffer, long location) {
  if (location < 0 || location >= BUFFER_SIZE) {
    (void) fprintf(stderr, "Location out of range\n");
    return;
  }
  print_hex(buffer + location, BYTES_PER_LINE);
}

void edit_command(unsigned char* buffer, char* input, long location) {
  if (location < 0 || location >= BUFFER_SIZE) {
    (void) fprintf(stderr, "Location out of range\n");
    return;
  }
  char* endptr = NULL;
  unsigned int hexValue = strtoul(input + 2, &endptr, HEX_BASE);
  if (*endptr != '\n' && *endptr != '\0') {
    (void) fprintf(stderr, "Invalid hex format\n");
    return;
  }
  buffer[location] = (unsigned char) hexValue;
}

void file_close(FILE* file) {
  if (fclose(file) != 0) {
    perror("Error closing file");
  }
}

void file_write(const char* filename, unsigned char* buffer, size_t bytes_read) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("Error opening file for writing");
    return;
  }
  if (fwrite(buffer, sizeof(unsigned char), bytes_read, file) != bytes_read) {
    perror("Error writing file");
    file_close(file);
    return;
  }
  file_close(file);
}

void process_commands(unsigned char* buffer, FILE* file, char** argv, size_t bytes_read) {
  char input[INPUT_SIZE];
  char cmd = ' ';
  char* ptr = NULL;
  char* endptr = NULL;
  long location = 0;

  while (true) {
    printf("\nEnter command: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
      if (feof(stdin)) {
        break;
      }
      perror("Error reading input");
      continue;
    }

    cmd = input[0];

    if (cmd == 'q') {
        file_close(file);
        file_write(argv[1], buffer, bytes_read);
        return;
    }

    ptr = input + 1;
    location = strtol(ptr, &endptr, BASE_10);

    // Check for errors in conversion
    if (ptr == endptr) {
      (void) fprintf(stderr, "No number found\n");
      continue;
    }
    if (*endptr != ' ' && *endptr != '\n' && *endptr != '\0') {
      (void) fprintf(stderr, "Invalid characters found\n");
      continue;
    }

    switch (cmd) {
      case 'p':
        print_command(buffer, location);
        break;
      case 'e':
        edit_command(buffer, input, location);
        break;
      default:
        (void) fprintf(stderr, "Invalid command\n");
        break;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    (void) fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  FILE* file = fopen(argv[1], "r");
  if (file == NULL) {
    perror("Error opening file for reading");
    return 1;
  }

  unsigned char buffer[BUFFER_SIZE];
  size_t bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file);
  if (ferror(file)) {
    perror("Error reading file");
    file_close(file);
    return 1;
  }

  print_hex(buffer, bytes_read);

  process_commands(buffer, file, argv, bytes_read);

  return 0;
}
