/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef Command_h
#define Command_h

#include "Argument.h" // Argument
#include "StringCLI.h"

extern "C" {
#include "c/cmd_types.h" // cmd
}

#define COMMAND_TEMPORARY false
#define COMMAND_PERSISTENT true

enum class CommandType { NORMAL, BOUNDLESS, SINGLE };

class Command {
    friend class SimpleCLI;

private:
    cmd *cmdPointer;
    bool persistent;

    void addCmd(Command &c);
    bool setComposite(bool composite = true);

public:
    Command(cmd *cmdPointer = NULL, bool persistent = COMMAND_PERSISTENT);
    Command(const Command &c);
    Command(Command &&c);
    ~Command();

    Command &operator=(const Command &c);
    Command &operator=(Command &&c);

    bool operator==(const Command &c) const;
    bool operator!=(const Command &c) const;

    operator bool() const;

    bool setCaseSensetive(bool caseSensetive = true);
    bool setCaseSensitive(bool caseSensitive = true);
    bool setCallback(uint32_t (*callback)(cmd *c));

    void setDescription(const char *description);

    Argument addArg(const char *name, const char *defaultValue);
    Argument addArg(const char *name);
    Argument addPosArg(const char *name, const char *defaultValue);
    Argument addPosArg(const char *name);
    Argument addFlagArg(const char *name, const char *defaultValue = "");

    Argument addArgument(const char *name, const char *defaultValue);
    Argument addArgument(const char *name);
    Argument addPositionalArgument(const char *name, const char *defaultValue);
    Argument addPositionalArgument(const char *name);
    Argument addFlagArgument(const char *name, const char *defaultValue = "");

    bool equals(String name) const;
    bool equals(const char *name) const;
    bool equals(const Command &c) const;

    String getName() const;
    int countArgs() const;

    Argument getArgument(int i = 0) const;
    Argument getArgument(const char *name) const;
    Argument getArgument(String name) const;
    Argument getArgument(const Argument &a) const;

    Argument getArg(int i = 0) const;
    Argument getArg(const char *name) const;
    Argument getArg(String name) const;
    Argument getArg(const Argument &a) const;

    CommandType getType() const;

    bool hasDescription() const;
    String getDescription() const;

    String toString(bool description = true) const;
    void toString(String &s, bool description = true) const;

    bool run() const;

    cmd *getPtr();

    Command addCmd(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addBoundlessCmd(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addSingleArgCmd(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addCompositeCmd(const char *name, uint32_t (*callback)(cmd *c) = NULL);

    Command addCommand(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addBoundlessCommand(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addSingleArgumentCommand(const char *name, uint32_t (*callback)(cmd *c) = NULL);
    Command addCompositeCommand(const char *name, uint32_t (*callback)(cmd *c) = NULL);
};

#endif /* ifndef Command_h */