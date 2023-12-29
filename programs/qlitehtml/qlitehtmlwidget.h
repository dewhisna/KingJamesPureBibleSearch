// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "qlitehtml_global.h"

#include <QAbstractScrollArea>
#include <QTextDocument>

#include <functional>

class QLiteHtmlWidgetPrivate;
class QMimeData;

class QLITEHTML_EXPORT QLiteHtmlWidget : public QAbstractScrollArea
{
	Q_OBJECT
public:
	explicit QLiteHtmlWidget(QWidget *parent = nullptr);
	~QLiteHtmlWidget() override;

	// declaring the getters Q_INVOKABLE to make them Squish-testable
	Q_INVOKABLE QUrl source() const;

	void setHtml(const QString &content);
	Q_INVOKABLE QString html() const;
	Q_INVOKABLE QString title() const;

	void setZoomFactor(qreal scale);
	qreal zoomFactor() const;

	bool findText(const QString &text,
				  QTextDocument::FindFlags flags,
				  bool incremental,
				  bool *wrapped = nullptr);

	void setDefaultFont(const QFont &font);
	QFont defaultFont() const;

	void setCSSExt(const QString &extcss);

	void scrollToAnchor(const QString &name);

	using ResourceHandler = std::function<QByteArray(QUrl)>;
	void setResourceHandler(const ResourceHandler &handler);

	// declaring this Q_INVOKABLE to make it Squish-testable
	Q_INVOKABLE QString selectedText() const;


	bool isBackwardAvailable() const;
	bool isForwardAvailable() const;
	void clearHistory();
	QString historyTitle(int i) const;
	QUrl historyUrl(int i) const;
	int backwardHistoryCount() const;
	int forwardHistoryCount() const;

public slots:
	void zoomIn(int range = 1);
	void zoomOut(int range = 1);

	void setSource(const QUrl &url);
	virtual void backward();
	virtual void forward();
	virtual void home();
	virtual void reload();

	void copy();
	void clear();

signals:
	void contextMenuRequested(const QPoint &pos, const QUrl &url);

// Note: this signal is used, and left here for reference, but it's
//		from QWidget and shouldn't be redefined here, as doing so
//		messes up the QMetaObject signal indexes.  selectionChanged
//		and copyAvailable were in that category, if we were to
//		inherit from QTextEdit instead of QAbstractScrollArea:
//	void customContextMenuRequested(const QPoint &pos);
	void selectionChanged();
	void copyAvailable(bool available);

	void backwardAvailable(bool available);
	void forwardAvailable(bool available);
	void historyChanged();
	void sourceChanged(const QUrl &name);
	void highlighted(const QUrl &url);
	void anchorClicked(const QUrl &url);

protected:
	virtual void paintEvent(QPaintEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void leaveEvent(QEvent *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;

	virtual void showEvent(QShowEvent *event) override;

	virtual QMimeData *createMimeDataFromSelection() const;

private:
	void updateHightlightedLink();
	void setHightlightedLink(const QUrl &url);
	void withFixedTextPosition(const std::function<void()> &action);
	void render();
	QPoint scrollPosition() const;
	void setScrollPosition(const QPoint &point);
	void htmlPos(const QPoint &pos, QPoint *viewportPos, QPoint *htmlPos) const;
	QPoint toVirtual(const QPoint &p) const;
	QSize toVirtual(const QSize &s) const;
	QRect toVirtual(const QRect &r) const;
	QRect fromVirtual(const QRect &r) const;

	friend class QLiteHtmlWidgetPrivate;

	void zoomInF(float range);

	QLiteHtmlWidgetPrivate *d;
};
