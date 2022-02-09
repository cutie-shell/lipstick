/***************************************************************************
**
** Copyright (c) 2012 Jolla Ltd.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef PULSEAUDIOCONTROL_H
#define PULSEAUDIOCONTROL_H

#include <pulse/pulseaudio.h>

#include <QObject>
/*!
 * \class PulseAudioControl
 *
 * \brief Gets and sets the volume using the MainVolume API.
 */
class PulseAudioControl : public QObject
{
    Q_OBJECT

public:
    //! Construct a PulseAudioControl instance
    PulseAudioControl(QObject *parent = NULL);

    //! Destroys the PulseAudioControl instance
    virtual ~PulseAudioControl();

signals:
    /*!
     * Sent when the current or maximum volume has changed.
     *
     * \param level The new volume level
     * \param maximum The maximum volume level
     */
    void volumeChanged(int volume, int maximum);

    /*!
     * Sent when main volume is set to so high that it can hurt hearing
     *
     * \param safeLevel Highest level for volume that does not risk hurting hearing
     */
    void highVolume(int safeLevel);

    /*!
     * Sent when user needs to be warned about long listening time.
     *
     * \param listeningTime listening time in minutes
     */
    void longListeningTime(int listeningTime);

    /*!
     * Sent when the call status has changed
     *
     * \param callActive \c true if a call is active, \c false otherwise
     */
    void callActiveChanged(bool callActive);

    void mediaStateChanged(const QString &state);


    void pulseConnectFailed();
    void defaultSourceIndexChanged();

public slots:
    /*!
     * Queries the PulseAudio daemon for the volume levels (current and maximum).
     * If successful, volumeChanged signal will be emitted.
     */
    void update();

    /*!
     * Changes the volume level through the volume backend.
     *
     * \param volume The desired volume level
     */
    void setVolume(int volume);

private slots:
    //! Follow PulseAudio visibility in sessionbus
    void pulseRegistered(const QString &service);
    void pulseUnregistered(const QString &service);

private:
    //! Opens connection to PulseAudio daemon.
    void openConnection();

    /*!
     * Stores the current volume and the maximum volume.
     *
     * \param currentStep The current volume step
     * \param stepCount Number of volume steps
     */
    void setSteps(quint32 currentStep, quint32 stepCount);

    //! Registers a signal handler to listen to the PulseAudio MainVolume1 StepsUpdated signal
    void addSignalMatch();
    Q_DISABLE_COPY(PulseAudioControl)

#ifdef UNIT_TEST
    friend class Ut_PulseAudioControl;
#endif
    int paVolume2Percent(pa_volume_t vol);
    pa_volume_t percent2PaVolume(int percent);

    static void stateCallBack(pa_context *context, void *userdata);
    static void subscribeCallBack(pa_context *context, pa_subscription_event_type_t t, uint32_t index, void *userdata);
    static void clientCallback(pa_context *, const pa_client_info *i, int eol, void *userdata);
    static void sinkCallBack(pa_context *, const pa_sink_info *i, int eol, void *userdata);
    static void sourceCallBack(pa_context *, const pa_source_info *i, int eol, void *userdata);
    static void sinkInputCallBack(pa_context *, const pa_sink_input_info *i, int eol, void *userdata);
    static void sourceOutputCallBack(pa_context *, const pa_source_output_info *i, int eol, void *userdata);
    static void serverInfoCallback(pa_context *, const pa_server_info *i, void *userdata);
    static void cardCallBack(pa_context *, const pa_card_info *i, int eol, void *userdata);

    pa_context* m_paContext;
    pa_mainloop_api* m_paAPI;
    void *userdata;

    QString m_defaultSinkName;
    QString m_defaultSourceName;

    int m_defaultSinkNameID;
    int m_defaultSourceNameID;

    QList<pa_sink_info> m_sinks;
};

#endif

