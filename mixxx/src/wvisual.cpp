/***************************************************************************
                          wvisual.cpp  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wvisual.h"
#include "visual/visualchannel.h"

WVisual::WVisual(QWidget *pParent, const char *pName, const QGLWidget *pShareWidget, QColor qBackground) : QGLWidget(pParent,pName,pShareWidget)
{
    m_pVisualController = new VisualController(qBackground);

    installEventFilter(this);
    m_qtTime.start();
    startTimer(25);

    m_qlList.setAutoDelete(false);
}

WVisual::~WVisual()
{
    // Stop timer
    killTimers();

    // Delete associated VisualChannels
    while (m_qlList.remove());

    // Finally delete the VisualController
    delete m_pVisualController;
}

bool WVisual::eventFilter(QObject *o, QEvent *e)
{
    // Handle mouse press events
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *m = (QMouseEvent *)e;

        m_iStartPosX = -1;
        if (m->button()==Qt::LeftButton)
        {
            // Store current x position of mouse pointer
            m_iStartPosX = m->x();
            emit(valueChangedLeftDown(64.));
        }
        else if (m->button()==Qt::RightButton)
        {
            // Toggle fish eye mode on each channel associated
            VisualChannel *c;
            for (c=m_qlList.first(); c; c=m_qlList.next())
                c->toggleFishEyeMode();
        }
    }
    else if (e->type() == QEvent::MouseMove)
    {
        // Only process mouse move if it was initiated by a left click
        if (m_iStartPosX!=-1)
        {
            QMouseEvent *m = (QMouseEvent *)e;
            int v = 64+m->x()-m_iStartPosX;
            if (v<0)
                v = 0;
            else if (v>127)
                v= 127;
            emit(valueChangedLeftDown((double)v));
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        emit(valueChangedLeftDown(64.));
    }

/*
        int id = m_Picking.pick(m->x(),m->y());
        qDebug("pick id %i",id);
        
        VisualChannel *c;
        for (c = m_qlList.first(); c; c = m_qlList.next())
            c->zoom(id);
    }
*/

    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
}

VisualChannel *WVisual::add(ControlPotmeter *pPlaypos)
{
    VisualChannel *c = new VisualChannel(pPlaypos, m_pVisualController);

    // Position coding... hack
    if (m_qlList.isEmpty())
    {
        c->setPosX(-176);
        c->setZoomPosX(50);
    }
    else    
    {
        c->setPosX(50);
        c->setZoomPosX(50);
    }
        
    m_qlList.append(c);
    return c;
}
    
void WVisual::initializeGL()
{
    m_pVisualController->init();
    //m_pVisualBackplane = new VisualBackplane();
//    controller->add(m_pVisualBackplane);

    m_Picking.init(m_pVisualController);
}


void WVisual::paintGL()
{
    // Get time since last paint, and reset timer
    int msec = m_qtTime.elapsed();
    m_qtTime.restart();

    // Update position of each channel
    VisualChannel *c;
    for (c = m_qlList.first(); c; c = m_qlList.next())
        c->move(msec);

    // Display stuff
    m_pVisualController->display();
}

void WVisual::resizeGL(int width, int height)
{
    m_pVisualController->resize((GLsizei)width,(GLsizei)height);
}

void WVisual::timerEvent(QTimerEvent*)
{
    updateGL();
}
