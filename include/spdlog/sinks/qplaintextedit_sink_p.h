// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <QObject>
#include <QPlainTextEdit>

namespace _spdlog_p {
namespace _sinks_p {
class qplaintextedit_sink_p : public QObject {
    Q_OBJECT
public:
    qplaintextedit_sink_p(QPlainTextEdit *textedit = nullptr) {
        if (textedit != nullptr) {
            textedit_ = textedit;
            connect(this, &qplaintextedit_sink_p::append_text, textedit_,
                    &QPlainTextEdit::appendPlainText);
        }
    }

    ~qplaintextedit_sink_p() {}

    void append(const std::string &str) {
        emit append_text(QString::fromStdString(str));
    }

signals:
    void append_text(const QString &);

private:
    QPlainTextEdit *textedit_ = nullptr;
};
} // namespace _sinks_p
} // namespace _spdlog_p
