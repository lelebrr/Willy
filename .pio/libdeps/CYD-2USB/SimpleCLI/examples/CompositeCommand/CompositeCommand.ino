/*
   Copyright (c) 2025 Rennan COckles
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/rennancockles/SimpleCLI
 */

// Include Library
#include <SimpleCLI.h>

// Create CLI Object
SimpleCLI cli;

// Commands
Command screen;
Command color;
Command brightness;

// Callback function for color subcommand
uint32_t colorCallback(cmd *c) {
    Command cmd(c); // Create wrapper object

    Argument arg = cmd.getArgument("color");
    String color = arg.getValue();

    // Print result
    Serial.println("Setting screen color to " + color);
    return true;
}

// Callback function for brightness subcommand
uint32_t brightnessCallback(cmd *c) {
    Command cmd(c); // Create wrapper object

    Argument arg = cmd.getArgument("value");
    String value = arg.getValue();

    // Print result
    Serial.println("Setting screen brightness to " + value + "%");
    return true;
}

// Callback in case of an error
void errorCallback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand()) {
        Serial.print("Did you mean \"");
        Serial.print(cmdError.getCommand().toString());
        Serial.println("\"?");
    }
}

void setup() {
    // Startup stuff
    Serial.begin(9600);
    delay(2000);
    Serial.println("Started!");

    cli.setOnError(errorCallback); // Set error Callback

    // Create the screen command
    screen = cli.addBoundlessCommand("screen");

    // Create the color subcommand
    // For example: screen color ffffff
    color = cli.addBoundlessCommand("col/or", colorCallback);
    color.addPosArg("color");

    // Create the color subcommand
    // For example: screen br 75
    brightness = cli.addBoundlessCommand("br/ight/ness", brightnessCallback);
    brightness.addPosArg("value");

    Serial.println("Type: screen color ffffff");
}

void loop() {
    // Check if user typed something into the serial monitor
    if (Serial.available()) {
        // Read out string from the serial monitor
        String input = Serial.readStringUntil('\n');

        Serial.print("# ");
        Serial.println(input);

        // Parse the user input into the CLI
        cli.parse(input);
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Serial.print("ERROR: ");
        Serial.println(cmdError.toString());

        if (cmdError.hasCommand()) {
            Serial.print("Did you mean \"");
            Serial.print(cmdError.getCommand().toString());
            Serial.println("\"?");
        }
    }
}