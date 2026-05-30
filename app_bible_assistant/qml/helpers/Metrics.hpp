#pragma once

#include "bibstd/util/non_owning_ptr.hpp"

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{

///
/// QML Metrics singleton.
/// This class provides application wide style metrics (spacing, font sizes, radii).
///
class Metrics final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  // Font sizes
  Q_PROPERTY(int fontSizeSmall MEMBER fontSizeSmall_ CONSTANT)
  Q_PROPERTY(int fontSizeBody MEMBER fontSizeBody_ CONSTANT)
  Q_PROPERTY(int fontSizeHeading MEMBER fontSizeHeading_ CONSTANT)

  // Spacing
  Q_PROPERTY(int spacingTiny MEMBER spacingTiny_ CONSTANT)
  Q_PROPERTY(int spacingSmall MEMBER spacingSmall_ CONSTANT)
  Q_PROPERTY(int spacingMedium MEMBER spacingMedium_ CONSTANT)
  Q_PROPERTY(int spacingLarge MEMBER spacingLarge_ CONSTANT)
  Q_PROPERTY(int spacingXLarge MEMBER spacingXLarge_ CONSTANT)

  // Radii
  Q_PROPERTY(int radiusMedium MEMBER radiusMedium_ CONSTANT)
  Q_PROPERTY(int radiusLarge MEMBER radiusLarge_ CONSTANT)

  // Sizes
  Q_PROPERTY(int controlHeight MEMBER controlHeight_ CONSTANT)

public: // Structors
  explicit Metrics(bibstd::util::non_owning_ptr<QObject> parent = nullptr);
  ~Metrics() noexcept override;

private: // Variables
  // Font sizes
  int fontSizeSmall_{8};
  int fontSizeBody_{9};
  int fontSizeHeading_{11};

  // Spacing
  int spacingTiny_{2};
  int spacingSmall_{4};
  int spacingMedium_{6};
  int spacingLarge_{8};
  int spacingXLarge_{12};

  // Radii
  int radiusMedium_{4};
  int radiusLarge_{8};

  // Sizes
  int controlHeight_{24};
};

} // namespace aba::qml
