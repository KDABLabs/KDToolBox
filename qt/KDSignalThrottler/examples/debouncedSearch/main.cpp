/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <KDSignalThrottler.h>

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto helpLabel = new QLabel(this);
        helpLabel->setWordWrap(true);
        helpLabel->setText(tr("<h2>Debouncer example</h2>"
                              "<p>Type some text in the line edit below.</p>"
                              "<p>The label below it will be updated only after 250ms of inactivity "
                              "on the line edit, due to a debouncer filtering the line edit's "
                              "signal emissions.</p>"));

        auto line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);

        m_edit = new QLineEdit(this);
        m_edit->setPlaceholderText(tr("Search with a delay"));

        m_label = new QLabel(this);

        auto debouncer = new KDToolBox::KDSignalDebouncer(this);
        debouncer->setTimeout(250);

        connect(m_edit, &QLineEdit::textChanged, debouncer, &KDToolBox::KDSignalDebouncer::throttle);
        connect(debouncer, &KDToolBox::KDSignalDebouncer::triggered, this, &Widget::updateLabel);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(helpLabel);
        layout->addWidget(line);
        layout->addWidget(m_edit);
        layout->addWidget(m_label);
        setLayout(layout);

        updateLabel();
    }

private:
    void updateLabel()
    {
        const auto text = m_edit->text();
        if (text.isEmpty())
            m_label->setText(tr("<h3>Nothing to search for.</h3>"));
        else
            m_label->setText(tr("<h3>Searching for: <b>\"%1\"</b>...</h3>").arg(m_edit->text()));
    }

    QLineEdit *m_edit;
    QLabel *m_label;
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("Debouncer example"));

    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}

#include "main.moc"
