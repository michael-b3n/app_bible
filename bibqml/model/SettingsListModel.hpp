#pragma once

#include <bibstd/util/non_owning_ptr.hpp>

#include <QAbstractListModel>
#include <QMetaEnum>
#include <QObject>
#include <QtQml/qqmlregistration.h>

#include <deque>
#include <memory>

// Forward declaration
namespace bibstd::workflow
{
class workflow_settings;
} // namespace bibstd::workflow

namespace bibqml
{

///
/// List model providing bible passages for a ListView.
/// Supports dynamic loading of previous/next verses as the user scrolls.
///
class SettingsListModel final : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

  // Typedefs
  ///
  /// ListModel Entry
  ///
  struct Entry final
  {};

  // Variables
  std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings_;
  std::deque<Entry> entries_;

public: // Typedefs
  enum Role
  {
    SomeRole = Qt::UserRole + 1,
  };
  Q_ENUM(Role)

public: // Structors
  explicit SettingsListModel(
    std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings,
    bibstd::util::non_owning_ptr<QObject> parent = nullptr
  );
  ~SettingsListModel() noexcept override;

public: // Overrides
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

public: // Modifiers
signals:
  void refreshed();

private: // Implementation
  Entry makeEntry() const;
};

} // namespace bibqml
