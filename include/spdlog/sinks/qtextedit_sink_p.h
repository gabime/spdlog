// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <QObject>
#include <QTextEdit>

namespace _spdlog_p {
namespace _sinks_p {
class qtextedit_sink_p : public QObject {
    Q_OBJECT
public:
    qtextedit_sink_p(QTextEdit *textedit = nullptr) {
        if (textedit != nullptr) {
            textedit_ = textedit;
            connect(this, &qtextedit_sink_p::append_text, textedit_,
                    &QTextEdit::append);
        }
    }

    ~qtextedit_sink_p() {}

    void append(const std::string &str) {
        emit append_text(QString::fromStdString(str));
    }

signals:
    void append_text(const QString &);

private:
    QTextEdit *textedit_ = nullptr;
};
} // namespace _sinks_p
} // namespace _spdlog_p
