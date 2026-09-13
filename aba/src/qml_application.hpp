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
///
auto load_qml_document(QQmlApplicationEngine& engine, QGuiApplication& app, const QString& url) -> void;

} // namespace aba
