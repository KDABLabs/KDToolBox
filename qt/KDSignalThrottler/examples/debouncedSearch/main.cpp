/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

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

        connect(m_edit, &QLineEdit::textChanged,
                debouncer, &KDToolBox::KDSignalDebouncer::throttle);
        connect(debouncer, &KDToolBox::KDSignalDebouncer::triggered,
                this, &Widget::updateLabel);

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
    QApplication::setApplicationName("Debouncer example");

    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}

#include "main.moc"
