# Plagiatkontrol

Plagiarism detection system to help students spot unintended plagiarism in their assignments

## Features

-   Read and compare contents of provided files
-   Remove unwanted characters from text when comparing
-   Split text into words
-   Detect language specific conjugations
-   Detect language specific semantic words
-   Detect indexes of words containing a quotation sign
-   Show calculated percentage of potentially plagiarised text
-   Print documents with colored text
    -   🔴 = Potentially plagiarised text
    -   🟡 = Potentially cited text

## Run Locally

Go to the project directory

```bash
  cd Plagiatkontrol
```

Compile and run program

```bash
  # macOS & Linux
  gcc program.c && ./a.out
```

```bash
  # Windows
  gcc program.c && a.out
```

Provide essential input - (you can provide the following inputs as the example files are located in the project)

```bash
  What language is your files written in? [1] English  [2] Danish: 1
  Input your filename: user.txt
  Input source filenames (divide mulitple filenames with a space): src_1.txt src_2.txt
```

## Running Tests

To run tests, run the following command

```bash
  # macOS & Linux
  gcc program.c && ./a.out --test
```

```bash
  # Windows
  gcc program.c && a.out --test
```
