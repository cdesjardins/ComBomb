#ifndef BACKTABEVENT_H
#define BACKTABEVENT_H

const QEvent::Type SendBackTab = static_cast<QEvent::Type>(QEvent::User + 1);

// Define your custom event subclass
class SendBackTabEvent : public QEvent
{
public:
    SendBackTabEvent():
        QEvent(SendBackTab)
    {
    }
};

#endif // BACKTABEVENT_H
