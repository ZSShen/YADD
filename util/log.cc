
#include "log.h"


void LogMessage::LogLine(const LogMessageData& data, const char* message)
{
    char severity = "VDIWEFF"[data.severity];
    fprintf(stderr, "%c %s:%d] %s\n", severity, data.file, data.line_number, message);
}

LogMessageData::LogMessageData(const char* file, int line, LogSeverity severity, int error)
    : file(file),
      line_number(line),
      severity(severity),
      error(error)
{
    const char* last_slash = strrchr(file, '/');
    file = (last_slash == NULL) ? file : last_slash + 1;
}

LogMessage::~LogMessage()
{
    if (data_->error != -1)
        data_->buffer << ": " << strerror(data_->error);
    std::string msg(data_->buffer.str());

    if (msg.find('\n') == std::string::npos)
        LogLine(*data_, msg.c_str());
    else {
        msg += '\n';
        size_t i = 0;
        while (i < msg.size()) {
            size_t nl = msg.find('\n', i);
            msg[nl] = '\0';
            LogLine(*data_, &msg[i]);
            i = nl + 1;
        }
    }

    if (data_->severity == FATAL)
        exit(EXIT_FAILURE);
}
