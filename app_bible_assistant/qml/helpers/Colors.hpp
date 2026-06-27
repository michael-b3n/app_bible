#pragma once

#include <QColor>
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{
namespace detail
{

///
/// Helper function to convert a hex color string to a QColor object.
/// The input string should be in the format "#RRGGBBAA" or "#RRGGBB".
/// \param color The hex color string to convert
/// \return The corresponding QColor object
///
auto toQColor(std::string color) -> QColor;

} // namespace detail

///
/// QML Colors singleton.
/// This class provides application wide style colors.
///
class Colors final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QColor backgroundTransparent MEMBER backgroundTransparent_ NOTIFY backgroundTransparentChanged)
  Q_PROPERTY(QColor backgroundSolid MEMBER backgroundSolid_ NOTIFY backgroundSolidChanged)
  Q_PROPERTY(QColor border MEMBER border_ NOTIFY borderChanged)
  Q_PROPERTY(QColor text MEMBER text_ NOTIFY textChanged)
  Q_PROPERTY(QColor hover MEMBER hover_ NOTIFY hoverChanged)
  Q_PROPERTY(QColor pressed MEMBER pressed_ NOTIFY pressedChanged)

  Q_PROPERTY(QColor verseBox MEMBER verseBox_ NOTIFY verseBoxChanged)
  Q_PROPERTY(QColor verseText MEMBER verseText_ NOTIFY verseTextChanged)
  Q_PROPERTY(QColor chapterText MEMBER chapterText_ NOTIFY chapterTextChanged)
  Q_PROPERTY(QColor bookHeader MEMBER bookHeader_ NOTIFY bookHeaderChanged)
  Q_PROPERTY(QColor bookHeaderText MEMBER bookHeaderText_ NOTIFY bookHeaderTextChanged)

public: // Structors
  explicit Colors(QObject* parent = nullptr);
  ~Colors() noexcept override;

signals:
  void backgroundTransparentChanged();
  void backgroundSolidChanged();
  void borderChanged();
  void textChanged();
  void hoverChanged();
  void pressedChanged();

  void verseBoxChanged();
  void verseTextChanged();
  void chapterBoxChanged();
  void chapterTextChanged();
  void bookHeaderChanged();
  void bookHeaderTextChanged();

private: // Variables
  QColor backgroundTransparent_{detail::toQColor("#fef5deed")};
  QColor backgroundSolid_{detail::toQColor("#fef5deff")};
  QColor border_{detail::toQColor("#937a7aff")};
  QColor text_{detail::toQColor("#000000ff")};
  QColor hover_{detail::toQColor("#eabca8ff")};
  QColor pressed_{detail::toQColor("#937a7aff")};

  QColor verseBox_{detail::toQColor("#937a7aff")};
  QColor verseText_{detail::toQColor("#FFFFFFff")};
  QColor chapterText_{detail::toQColor("#F0E6FFff")};
  QColor bookHeader_{detail::toQColor("#c2ada7ff")};
  QColor bookHeaderText_{detail::toQColor("#FFFFFFff")};
};

} // namespace aba::qml
