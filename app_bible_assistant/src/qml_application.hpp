#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QString>

namespace aba
{

///
/// Configure how the qml layer is rendered and styled.
/// \note Must be called before the qml application is constructed.
///
auto configure_qml_layer() -> void;

///
/// Load a qml document and exit the application if its root object cannot be created.
/// \param engine The QQmlApplicationEngine instance
/// \param app The QGuiApplication instance
/// \param url The url of the qml document to load
///
auto load_qml_document(QQmlApplicationEngine& engine, QGuiApplication& app, const QString& url) -> void;

} // namespace aba
