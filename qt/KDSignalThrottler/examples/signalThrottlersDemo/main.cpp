/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <KDSignalThrottler.h>

#include <QApplication>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include <QFormLayout>
#include <QVBoxLayout>

#include <QPainter>

#include <deque>

class DrawSignalsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawSignalsWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        setAttribute(Qt::WA_OpaquePaintEvent);

        m_scrollTimer = new QTimer(this);
        m_scrollTimer->setInterval(10);
        connect(m_scrollTimer, &QTimer::timeout, this, &DrawSignalsWidget::scroll);
        m_scrollTimer->start();
    }

    QSize sizeHint() const override { return QSize(400, 200); }

    void addSignalActivation() { m_signalActivations.push_front(0); }

    void addThrottledSignalActivation() { m_throttledSignalActivations.push_front(0); }

private:
    void scroll()
    {
        const int cutoff = width();
        scrollAndCut(&m_signalActivations, cutoff);
        scrollAndCut(&m_throttledSignalActivations, cutoff);

        update();
    }

    static void scrollAndCut(std::deque<int> *v, int cutoff)
    {
        auto i = v->begin();
        const auto e = v->end();
        for (; i != e; ++i)
        {
            ++*i;
            if (*i > cutoff)
                break;
        }
        v->erase(i, e);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), Qt::white);

        const int h = height();
        const int h2 = h / 2;
        const int w = width();

        drawSignals(&p, m_signalActivations, Qt::red, 0, h2);
        drawSignals(&p, m_throttledSignalActivations, Qt::blue, h2, h);

        {
            p.drawText(QRect(0, 0, w, h2), Qt::AlignRight | Qt::AlignVCenter, tr("Source signal"));
            p.drawText(QRect(0, h2, w, h2), Qt::AlignRight | Qt::AlignVCenter, tr("Throttled signal"));
        }

        {
            p.save();
            QPen pen;
            pen.setWidthF(2.0);
            p.drawLine(0, h2, w, h2);
            p.restore();
        }
    }

    static void drawSignals(QPainter *p, const std::deque<int> &v, const QColor &color, int yStart, int yEnd)
    {
        p->save();

        QPen pen;
        pen.setWidthF(2.0);
        pen.setColor(color);
        p->setPen(pen);

        for (int i : v)
            p->drawLine(i, yStart, i, yEnd);

        p->restore();
    }

    QTimer *m_scrollTimer;
    std::deque<int> m_signalActivations;
    std::deque<int> m_throttledSignalActivations;
};

class DemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        createUi();

        // throttler setup
        connect(m_throttlerKindComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
                &DemoWidget::createThrottler);
        createThrottler();

        connect(m_throttlerTimeoutSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
                &DemoWidget::setThrottlerTimeout);
        setThrottlerTimeout();

        // button -> signal to be throttled
        connect(m_mainButton, &QPushButton::clicked, this, &DemoWidget::signalToBeThrottled);

        // auto-trigger setup
        m_autoTriggerTimer = new QTimer(this);
        m_autoTriggerTimer->setTimerType(Qt::PreciseTimer);
        connect(m_autoTriggerCheckBox, &QCheckBox::clicked, this, &DemoWidget::startOrStopAutoTriggerTimer);
        startOrStopAutoTriggerTimer();

        connect(m_autoTriggerIntervalSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
                &DemoWidget::setAutoTriggerTimeout);
        setAutoTriggerTimeout();

        connect(m_autoTriggerTimer, &QTimer::timeout, this, &DemoWidget::signalToBeThrottled);

        // our signal -> drawing widget
        connect(this, &DemoWidget::signalToBeThrottled, m_drawSignalsWidget, &DrawSignalsWidget::addSignalActivation);

        // the connections from and to the throttler are done in createThrottler.
    }

Q_SIGNALS:
    void signalToBeThrottled();

private:
    void createUi()
    {
        auto helpLabel = new QLabel(this);
        helpLabel->setWordWrap(true);
        helpLabel->setText(tr("<h2>KDSignalThrottler example</h2>"
                              "<p>This example demonstrates the differences between "
                              "the different kinds of signal throttlers and debouncers."));

        auto throttlerKindGroupBox = new QGroupBox(tr("Throttler configuration"), this);
        {
            m_throttlerKindComboBox = new QComboBox(throttlerKindGroupBox);
            m_throttlerKindComboBox->addItems({tr("Throttler, trailing"), tr("Throttler, leading"),
                                               tr("Debouncer, trailing"), tr("Debouncer, leading")});

            m_throttlerTimeoutSpinBox = new QSpinBox(throttlerKindGroupBox);
            m_throttlerTimeoutSpinBox->setRange(1, 5000);
            m_throttlerTimeoutSpinBox->setValue(500);
            m_throttlerTimeoutSpinBox->setSuffix(tr(" ms"));

            auto layout = new QFormLayout(throttlerKindGroupBox);
            layout->addRow(tr("Kind of throttler:"), m_throttlerKindComboBox);
            layout->addRow(tr("Timeout:"), m_throttlerTimeoutSpinBox);
            throttlerKindGroupBox->setLayout(layout);
        }

        auto buttonGroupBox = new QGroupBox(tr("Throttler activation"));
        {
            m_mainButton = new QPushButton(tr("Press me!"), buttonGroupBox);

            m_autoTriggerCheckBox = new QCheckBox(tr("Trigger automatically"));

            auto autoTriggerLayout = new QHBoxLayout;
            {
                m_autoTriggerLabel = new QLabel(tr("Interval"), buttonGroupBox);
                m_autoTriggerIntervalSpinBox = new QSpinBox(buttonGroupBox);
                m_autoTriggerIntervalSpinBox->setRange(1, 5000);
                m_autoTriggerIntervalSpinBox->setValue(100);
                m_autoTriggerIntervalSpinBox->setSuffix(tr(" ms"));

                autoTriggerLayout->setContentsMargins(0, 0, 0, 0);
                autoTriggerLayout->addWidget(m_autoTriggerLabel);
                autoTriggerLayout->addWidget(m_autoTriggerIntervalSpinBox);
            }

            auto layout = new QVBoxLayout(buttonGroupBox);
            layout->addWidget(m_mainButton);
            layout->addWidget(m_autoTriggerCheckBox);
            layout->addLayout(autoTriggerLayout);
            buttonGroupBox->setLayout(layout);
        }

        auto resultGroupBox = new QGroupBox(tr("Result"));
        {
            m_drawSignalsWidget = new DrawSignalsWidget(resultGroupBox);
            auto layout = new QVBoxLayout(resultGroupBox);
            layout->addWidget(m_drawSignalsWidget);
            resultGroupBox->setLayout(layout);
        }

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(helpLabel);
        layout->addWidget(throttlerKindGroupBox);
        layout->addWidget(buttonGroupBox);
        layout->addWidget(resultGroupBox);

        setLayout(layout);
    }

    void createThrottler()
    {
        delete m_throttler;

        const int index = m_throttlerKindComboBox->currentIndex();
        switch (index)
        {
        case 0:
            m_throttler = new KDToolBox::KDSignalThrottler(this);
            break;
        case 1:
            m_throttler = new KDToolBox::KDSignalLeadingThrottler(this);
            break;
        case 2:
            m_throttler = new KDToolBox::KDSignalDebouncer(this);
            break;
        case 3:
            m_throttler = new KDToolBox::KDSignalLeadingDebouncer(this);
            break;
        }

        Q_ASSERT(m_throttler);

        m_throttler->setTimerType(Qt::PreciseTimer);
        connect(this, &DemoWidget::signalToBeThrottled, m_throttler, &KDToolBox::KDGenericSignalThrottler::throttle);
        connect(m_throttler, &KDToolBox::KDGenericSignalThrottler::triggered, m_drawSignalsWidget,
                &DrawSignalsWidget::addThrottledSignalActivation);

        setThrottlerTimeout();
    }

    void setThrottlerTimeout() { m_throttler->setTimeout(m_throttlerTimeoutSpinBox->value()); }

    void startOrStopAutoTriggerTimer()
    {
        const bool shouldStart = m_autoTriggerCheckBox->isChecked();
        if (shouldStart)
        {
            m_autoTriggerTimer->start();
        }
        else
        {
            m_autoTriggerTimer->stop();
        }

        m_autoTriggerIntervalSpinBox->setEnabled(shouldStart);
        m_autoTriggerLabel->setEnabled(shouldStart);
    }

    void setAutoTriggerTimeout()
    {
        const int timeout = m_autoTriggerIntervalSpinBox->value();
        m_autoTriggerTimer->setInterval(timeout);
    }

    QComboBox *m_throttlerKindComboBox;
    QSpinBox *m_throttlerTimeoutSpinBox;

    QPushButton *m_mainButton;

    QCheckBox *m_autoTriggerCheckBox;
    QLabel *m_autoTriggerLabel;
    QSpinBox *m_autoTriggerIntervalSpinBox;
    QTimer *m_autoTriggerTimer;

    DrawSignalsWidget *m_drawSignalsWidget;

    KDToolBox::KDGenericSignalThrottler *m_throttler = nullptr;
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KDSignalThrottler demo"));

    QApplication app(argc, argv);
    DemoWidget w;
    w.resize(600, 600);
    w.show();
    return app.exec();
}

#include "main.moc"
