/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */

#include "XPDFRenderer.h"

#include <QtGui>

#include <frameworks/UBPlatformUtils.h>

#include "core/memcheck.h"
#include <poppler-qt5.h>

XPDFRenderer::XPDFRenderer(const QString &filename, bool importingFile)
    : mDocument(0)
{
    Q_UNUSED(importingFile);

    mDocument = Poppler::Document::load(filename);
}

XPDFRenderer::~XPDFRenderer()
{
    if (mDocument)
    {
        delete mDocument;
    }
}

bool XPDFRenderer::isValid() const
{
    if (mDocument)
    {
        return ! mDocument->isLocked();
    }
    else
    {
        return false;
    }
}

int XPDFRenderer::pageCount() const
{
    if (isValid())
        return mDocument->numPages();

    return 0;
}

QString XPDFRenderer::title() const
{
    if (isValid())
    {
        return mDocument->title();
    }

    return QString();
}



QSizeF XPDFRenderer::pageSizeF(int pageNumber) const
{
    // BUG : this->dpiForRendering is 0.0
    //qreal dpiFactor = this->dpiForRendering / 72.0;
    qreal dpiFactor = 1.0;

    if (isValid())
    {
        Poppler::Page* pdfPage = mDocument->page(pageNumber - 1);

        QSizeF pdfPageSizeF = pdfPage->pageSizeF();
        pdfPageSizeF *= dpiFactor;

        return pdfPageSizeF;
    }

    return QSizeF(0.0, 0.0);
}


int XPDFRenderer::pageRotation(int pageNumber) const
{

    if (isValid())
    {
        Poppler::Page* pdfPage = mDocument->page(pageNumber - 1);
        Poppler::Page::Orientation pdfPageOrientation = pdfPage->orientation();

        if ( pdfPageOrientation == Poppler::Page::Orientation::Portrait )
            return 0;

        else if( pdfPageOrientation == Poppler::Page::Orientation::Landscape )
            return 90;

        else if( pdfPageOrientation == Poppler::Page::Orientation::UpsideDown )
            return 180;

        else
            return 270;

    }
    else
        return 0;
}

void XPDFRenderer::render(QPainter *p, int pageNumber, const QRectF &bounds)
{
    if (isValid())
    {

        Poppler::Page* pdfPage = mDocument->page(pageNumber - 1);

        QTransform savedTransform = p->worldTransform();
        QTransform withoutScaleTransform(1.0,
                                         savedTransform.m12(),
                                         savedTransform.m13(),
                                         savedTransform.m21(),
                                         1.0,
                                         savedTransform.m23(),
                                         savedTransform.m31(),
                                         savedTransform.m32(),
                                         savedTransform.m33());

        p->setWorldTransform(withoutScaleTransform);

        QRectF pdfPageRectF = QRectF(QPointF(0.0, 0.0), pageSizeF(pageNumber));
        if( bounds.isNull() || bounds.contains(pdfPageRectF) )
        {
            // BUG : this->dpiForRendering is 0.0
            //QImage image = pdfPage->renderToImage(this->dpiForRendering*savedTransform.m11(),this->dpiForRendering*savedTransform.m22());
            QImage image = pdfPage->renderToImage(72.0*savedTransform.m11(),72.0*savedTransform.m22());
            p->drawImage(QPointF(0.0,0.0), image);
        }
        else
        {
            QRectF pdfBounds = bounds.intersected(pdfPageRectF);
            // BUG : this->dpiForRendering is 0.0
            //QImage image = pdfPage->renderToImage(this->dpiForRendering*savedTransform.m11(),
            //                                      this->dpiForRendering*savedTransform.m22(),
            //                                      pdfBounds.x(),
            //                                      pdfBounds.y(),
            //                                      pdfBounds.width(),
            //                                      pdfBounds.height());
            QImage image = pdfPage->renderToImage(72.0*savedTransform.m11(),
                                                  72.0*savedTransform.m22(),
                                                  pdfBounds.x(),
                                                  pdfBounds.y(),
                                                  pdfBounds.width(),
                                                  pdfBounds.height());
            p->drawImage(QPointF(pdfBounds.width()*savedTransform.m11(),pdfBounds.height()*savedTransform.m22()), image);
        }

        p->setWorldTransform(savedTransform);
    }
}

