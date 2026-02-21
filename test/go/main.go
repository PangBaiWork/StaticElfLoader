package main

import (
    "fmt"
    "os"
)


func main() {
    fmt.Println("Hello from static Go ELF!")
    fmt.Println("This is statically linked.")
    if len(os.Args) < 2 {
        fmt.Fprintf(os.Stderr, "Usage: %s <message>\n", os.Args[0])
        os.Exit(1)
    }

    fmt.Printf("You said: %s\n", os.Args[1])
}