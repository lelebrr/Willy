#include "sound_commands.h"
#include "core/sd_functions.h"
#include "modules/others/audio.h"
#include <globals.h>

uint32_t toneCallback(cmd *c) {
  Command cmd(c);

  Argument freqArg = cmd.getArgument("frequency");
  Argument durArg = cmd.getArgument("duration");
  String strFreq = freqArg.getValue();
  String strDur = durArg.getValue();
  strFreq.trim();
  strDur.trim();

  bool soundEnabled = willyConfig.soundEnabled;
  willyConfig.soundEnabled = true;

  unsigned long frequency = std::stoul(strFreq.c_str());
  unsigned long duration = std::stoul(strDur.c_str());

  serialDevice->println((int)500UL);
  serialDevice->println((int)frequency);
  serialDevice->println((int)duration);

  _tone(frequency, duration);

  willyConfig.soundEnabled = soundEnabled;
  return true;
}

uint32_t playCallback(cmd *c) {
  // RTTTL player
  // music_player
  // mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6

  // File player
  // music_player boot.wav

  Command cmd(c);

  Argument arg = cmd.getArgument("song");
  String song = arg.getValue();
  song.trim();

  bool soundEnabled = willyConfig.soundEnabled;
  willyConfig.soundEnabled = true;

  if (song.indexOf(":") != -1)
    return playAudioRTTTLString(song);

  if (song.indexOf(".") != -1) {
    if (!song.startsWith("/"))
      song = "/" + song;

    FS *fs;
    if (!getFsStorage(fs))
      return false;

    if (!(*fs).exists(song)) {
      serialDevice->println("Song file does not exist");
      return false;
    }

    return playAudioFile(fs, song);
  }

  willyConfig.soundEnabled = soundEnabled;
  return false;
}

uint32_t ttsCallback(cmd *c) {
  // tts hello world

  Command cmd(c);

  Argument arg = cmd.getArgument(0);
  String text = arg.getValue();
  text.trim();

  bool soundEnabled = willyConfig.soundEnabled;
  willyConfig.soundEnabled = true;

  bool r = tts(text);

  willyConfig.soundEnabled = soundEnabled;
  return r;
}

void createSoundCommands(SimpleCLI *cli) {
  Command toneCmd = cli->addCommand("tone,beep", toneCallback);
  toneCmd.addPosArg("frequency", "500UL");
  toneCmd.addPosArg("duration", "500UL");

#ifdef HAS_NS4168_SPKR
  Command playCmd = cli->addCommand("play,music_player", playCallback);
  playCmd.addPosArg("song");

  Command ttsCmd = cli->addSingleArgCmd("tts,say", ttsCallback);
#endif

  // WebRadio command - placeholder for future implementation
  // Requires ESP8266Audio library integration
  Command webradioCmd = cli->addCommand("webradio", webradioCallback);
  webradioCmd.addPosArg("url");
}

// WebRadio implementation
uint32_t webradioCallback(cmd *c) {
  Command cmd(c);

  Argument urlArg = cmd.getArgument("url");
  String url = urlArg.getValue();
  url.trim();

  if (url == "") {
    serialDevice->println("Usage: webradio <url>");
    serialDevice->println(
        "Example: webradio http://stream.example.com/radio.mp3");
    return false;
  }

  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    serialDevice->println("ERROR: URL must start with http:// or https://");
    return false;
  }

  serialDevice->print("Starting WebRadio from: ");
  serialDevice->println(url);

  bool result = playAudioUrl(url, PLAYBACK_ASYNC);
  if (result) {
    serialDevice->println("WebRadio started successfully");
    serialDevice->println("Use 'stop' command to stop playback");
  } else {
    serialDevice->println("ERROR: Failed to start WebRadio");
  }

  return result;
}
