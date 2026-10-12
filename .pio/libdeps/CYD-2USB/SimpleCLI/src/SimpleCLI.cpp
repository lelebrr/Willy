/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "SimpleCLI.h"

extern "C" {
#include "c/cmd.h"       // cmd
#include "c/cmd_error.h" // cmd_error_destroy
#include "c/parser.h"    // parse_lines
}

SimpleCLI::SimpleCLI(int commandQueueSize, int errorQueueSize)
    : commandQueueSize(commandQueueSize), errorQueueSize(errorQueueSize) {}

SimpleCLI::~SimpleCLI() {
    cmd_destroy_rec(cmdList);
    cmd_destroy_rec(cmdQueue);
    cmd_error_destroy_rec(errorQueue);
}

void SimpleCLI::pause() { pauseParsing = true; }

void SimpleCLI::unpause() {
    pauseParsing = false;

    // Go through queued errors
    while (onError && errored()) { onError(getError().getPtr()); }

    // Go through queued commands
    if (available()) {
        cmd *prev = NULL;
        cmd *current = cmdQueue;
        cmd *next = NULL;

        while (current) {
            next = current->next;

            // Has callback, then run it and remove from queue
            if (current->callback) {
                current->callback(current);
                if (prev) prev->next = next;
                cmd_destroy(current);
            } else {
                prev = current;
            }

            current = next;
        }
    }
}

bool SimpleCLI::parse(const String &input) { return parse(input.c_str(), input.length()); }

bool SimpleCLI::parse(const char *input) {
    if (input) return parse(input, strlen(input));
    return false;
}

bool SimpleCLI::parse(const char *str, size_t len, cmd *commands) {
    // Split str into a list of lines
    line_list *l = parse_lines(str, len);

    // Go through all lines and try to find a matching command
    line_node *n = l->first;

    bool anyError = false;

    while (n) {
        cmd *h;
        if (commands) h = commands;
        else h = cmdList;
        bool success = false;
        bool errored = false;
        bool callbackSuccess = true;
        bool compositeSuccess = true;

        while (h && !success && !errored) {
            cmd_error *e = cmd_parse(h, n);

            // When parsing was successful
            if (e->mode == CMD_PARSE_SUCCESS) {
                if (h->composite) {
                    compositeSuccess =
                        parse(n->words->first->next->str, n->len - n->words->first->len - 1, h->cmd_list);
                }

                if (h->callback && !pauseParsing) callbackSuccess = h->callback(h);
                else cmdQueue = cmd_push(cmdQueue, cmd_copy(h), commandQueueSize);

                success = true;
            }

            // When command name matches but something else went wrong, exit with error
            else if (e->mode > CMD_NOT_FOUND) {
                if (onError && !pauseParsing) {
                    onError(e);
                } else {
                    errorQueue = cmd_error_push(errorQueue, cmd_error_copy(e), errorQueueSize);
                }

                errored = true;
            }

            // When command name does not match

            /*else (e->mode <= CMD_NOT_FOUND) {

            }*/

            cmd_error_destroy(e);

            cmd_reset_cli(h);

            h = h->next;
        }

        // No error but no success either => Command could not be found
        if (!errored && !success) {
            cmd_error *e = cmd_error_create_not_found(NULL, n->words->first);

            if (onError && !pauseParsing) {
                onError(e);
            } else {
                errorQueue = cmd_error_push(errorQueue, cmd_error_copy(e), errorQueueSize);
            }

            cmd_error_destroy(e);

            errored = true;
        }

        // Failed executing callback
        if (callbackFailureAsError && !callbackSuccess) {
            cmd_error *e = cmd_error_create_callback_failure(h);

            if (onError && !pauseParsing) {
                onError(e);
            } else {
                errorQueue = cmd_error_push(errorQueue, cmd_error_copy(e), errorQueueSize);
            }

            cmd_error_destroy(e);

            errored = true;
        }

        if (errored || !callbackSuccess || !compositeSuccess) anyError = true;

        n = n->next;
    }

    line_list_destroy(l);

    return !anyError;
}

bool SimpleCLI::available() const { return cmdQueue; }

bool SimpleCLI::errored() const { return errorQueue; }

bool SimpleCLI::paused() const { return pauseParsing; }

int SimpleCLI::countCmdQueue() const {
    cmd *h = cmdQueue;
    int i = 0;

    while (h) {
        h = h->next;
        ++i;
    }

    return i;
}

int SimpleCLI::countErrorQueue() const {
    cmd_error *h = errorQueue;
    int i = 0;

    while (h) {
        h = h->next;
        ++i;
    }

    return i;
}

Command SimpleCLI::getCmd() {
    if (!cmdQueue) return Command();

    // "Pop" cmd pointer from queue
    cmd *ptr = cmdQueue;

    cmdQueue = cmdQueue->next;

    // Create wrapper class and copy cmd
    Command c(ptr, COMMAND_TEMPORARY);

    // Destroy old cmd from queue
    cmd_destroy(ptr);

    return c;
}

Command SimpleCLI::getCmd(String name) { return getCmd(name.c_str()); }

Command SimpleCLI::getCmd(const char *name) {
    if (name) {
        cmd *h = cmdList;

        while (h) {
            if (cmd_name_equals(h, name, strlen(name), h->case_sensetive) == CMD_NAME_EQUALS)
                return Command(h);
            h = h->next;
        }
    }
    return Command();
}

Command SimpleCLI::getCommand() { return getCmd(); }

Command SimpleCLI::getCommand(String name) { return getCmd(name); }

Command SimpleCLI::getCommand(const char *name) { return getCmd(name); }

CommandError SimpleCLI::getError() {
    if (!errorQueue) return CommandError();

    // "Pop" cmd_error pointer from queue
    cmd_error *ptr = errorQueue;
    errorQueue = errorQueue->next;

    // Create wrapper class and copy cmd_error
    CommandError e(ptr, COMMAND_ERROR_TEMPORARY);

    // Destroy old cmd_error from queue
    cmd_error_destroy(ptr);

    return e;
}

void SimpleCLI::addCmd(Command &c) {
    if (!cmdList) {
        cmdList = c.cmdPointer;
    } else {
        cmd *h = cmdList;

        while (h->next) h = h->next;
        h->next = c.cmdPointer;
    }

    c.setCaseSensetive(caseSensetive);
    c.persistent = true;
}

Command SimpleCLI::addCmd(const char *name, uint32_t (*callback)(cmd *c)) {
    Command c(cmd_create_default(name));

    c.setCallback(callback);
    addCmd(c);

    return c;
}

Command SimpleCLI::addBoundlessCmd(const char *name, uint32_t (*callback)(cmd *c)) {
    Command c(cmd_create_boundless(name));

    c.setCallback(callback);
    addCmd(c);

    return c;
}

Command SimpleCLI::addSingleArgCmd(const char *name, uint32_t (*callback)(cmd *c)) {
    Command c(cmd_create_single(name));

    c.setCallback(callback);
    addCmd(c);

    return c;
}

Command SimpleCLI::addCompositeCmd(const char *name, uint32_t (*callback)(cmd *c)) {
    Command c(cmd_create_boundless(name));

    c.setComposite(true);
    c.setCallback(callback);
    addCmd(c);

    return c;
}

Command SimpleCLI::addCommand(const char *name, uint32_t (*callback)(cmd *c)) {
    return addCmd(name, callback);
}

Command SimpleCLI::addBoundlessCommand(const char *name, uint32_t (*callback)(cmd *c)) {
    return addBoundlessCmd(name, callback);
}

Command SimpleCLI::addSingleArgumentCommand(const char *name, uint32_t (*callback)(cmd *c)) {
    return addSingleArgCmd(name, callback);
}

Command SimpleCLI::addCompositeCommand(const char *name, uint32_t (*callback)(cmd *c)) {
    return addCompositeCmd(name, callback);
}

String SimpleCLI::toString(bool descriptions) const {
    String s;

    toString(s, descriptions);
    return s;
}

void SimpleCLI::toString(String &s, bool descriptions) const {
    cmd *h = cmdList;

    while (h) {
        Command(h).toString(s, descriptions);
        if (descriptions) s += "\r\n";
        s += "\r\n";
        h = h->next;
    }
}

void SimpleCLI::setCaseSensetive(bool caseSensetive) {
    this->caseSensetive = caseSensetive;

    cmd *h = cmdList;

    while (h) {
        h->case_sensetive = caseSensetive;
        h = h->next;
    }
}

void SimpleCLI::setCaseSensitive(bool caseSensitive) { setCaseSensetive(caseSensitive); }

void SimpleCLI::setCallbackFailureAsError(bool value) { this->callbackFailureAsError = value; }

void SimpleCLI::setOnError(void (*onError)(cmd_error *e)) { this->onError = onError; }

void SimpleCLI::setErrorCallback(void (*onError)(cmd_error *e)) { setOnError(onError); }
