#include "Translations.hpp"

#include <bibstd/util/contains.hpp>
#include <bibstd/util/log.hpp>

#include <algorithm>
#include <utility>

namespace aba::qml
{
namespace
{

// Instance the QML layer operates on. It is registered by the constructor of the
// application owned instance and reset when that instance is destroyed.
bibstd::util::non_owning_ptr<Translations> instance_{nullptr};

} // anonymous namespace

///
///
bibstd::util::non_owning_ptr<Translations> Translations::instance()
{
  return instance_;
}

///
///
Translations* Translations::create([[maybe_unused]] QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
  if(!instance_)
  {
    LOG_ERROR("translations do not exist: pretty names are displayed as identifiers");
    return nullptr;
  }
  // The instance is owned by the application, the QML engine must not delete it.
  if(jsEngine)
  {
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
  }
  return instance_;
}

///
///
Translations::Translations(pretty_names names, const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject{parent}
  , names_{std::move(names)}
  , language_{names_.languages().empty() ? QString{} : QString::fromStdString(names_.languages().front())}
{
  if(instance_)
  {
    LOG_ERROR("translations already exist: qml will operate on the first instance");
    return;
  }
  instance_ = this;
}

///
///
Translations::~Translations() noexcept
{
  if(instance_ == this)
  {
    instance_ = nullptr;
  }
}

///
///
QString Translations::language() const
{
  return language_;
}

///
///
QStringList Translations::availableLanguages() const
{
  auto result = QStringList{};
  result.reserve(static_cast<int>(names_.languages().size()));
  std::ranges::for_each(names_.languages(), [&](const auto& l) { result.append(QString::fromStdString(l)); });
  return result;
}

///
///
void Translations::setLanguage(const QString& language)
{
  if(language_ == language)
  {
    return;
  }
  if(!bibstd::util::contains(names_.languages(), language.toStdString()))
  {
    LOG_ERROR("set unknown language failed: language=\"{}\"", language.toStdString());
    return;
  }
  language_ = language;
  LOG_DEBUG("set language: language=\"{}\"", language_.toStdString());
  emit languageChanged();
}

///
///
QString Translations::name(const QString& key, const QString& language) const
{
  const auto lang = language.isEmpty() ? language_ : language;
  const auto name = names_.name(lang.toStdString(), key.toStdString());
  return name ? QString::fromStdString(*name) : key;
}

} // namespace aba::qml
