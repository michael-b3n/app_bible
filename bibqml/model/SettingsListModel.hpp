#pragma once

#include <bibstd/signal/synchronized_executor.hpp>
#include <bibstd/util/non_owning_ptr.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <QAbstractListModel>
#include <QList>
#include <QMetaEnum>
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QVariant>

#include <memory>
#include <vector>

namespace bibqml
{

///
/// List model providing settings for a ListView.
///
class SettingsListModel final : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

  // Typedefs
  ///
  /// Enum for the setting value type.
  ///
  enum ValueType_
  {
    BoolValueType,
    IntValueType,
    DoubleValueType,
    StringValueType,
    TimeValueType,
    PathValueType
  };

  ///
  /// Enum for the setting value wrapper type (none, optional, vector).
  ///
  enum WrapperType_
  {
    NoneWrapperType,
    OptionalWrapperType,
    ListWrapperType,
  };

  ///
  /// Enum for the setting validator type (unbound, range, list).
  ///
  enum ValidatorType_
  {
    UnboundValidatorType,
    RangeValidatorType,
    ListValidatorType,
  };

  ///
  /// ListModel Entry
  ///
  struct Entry final
  {
    // Role Variables
    std::string path;
    ValueType_ valueType;
    WrapperType_ wrapperType;
    ValidatorType_ validatorType;
    bibstd::workflow::workflow_settings::setting_type_erased_non_owning_ptr_variant_type setting;
  };

  // Variables
  std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings_;
  std::vector<Entry> entries_;
  bibstd::signal::synchronized_executor executor_;

public: // Typedefs
  using ValueType = ValueType_;
  Q_ENUM(ValueType)
  using WrapperType = WrapperType_;
  Q_ENUM(WrapperType)
  using ValidatorType = ValidatorType_;
  Q_ENUM(ValidatorType)

  enum Role
  {
    PathRole = Qt::UserRole + 1,
    ValueTypeRole,
    WrapperTypeRole,
    ValidatorTypeRole,
    ValueRole,
    ListValidatorDataRole
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
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  void disconnect();

private: // Implementation
  void addEntry(std::string path, const auto& setting);
};

} // namespace bibqml
