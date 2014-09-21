#ifndef CXXMIGEMO_H_
#define CXXMIGEMO_H_

#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "migemo.h"

namespace cxxmigemo {

class Migemo {
 private:
  class RegexpStringDeleter;
  using uchar = unsigned char;

 public:
  enum class DataFileType {
    Hankaku2Zenkaku = MIGEMO_DICTID_HAN2ZEN,
    Hiragana2Katakana = MIGEMO_DICTID_HIRA2KATA,
    MigemoDict = MIGEMO_DICTID_MIGEMO,
    Roman2Hiragana = MIGEMO_DICTID_ROMA2HIRA,
    Zenkaku2Hankaku = MIGEMO_DICTID_ZEN2HAN,
  };

  enum class OperatorType {
    CharClassIn = MIGEMO_OPINDEX_SELECT_IN,
    CharClassOut = MIGEMO_OPINDEX_SELECT_OUT,
    GroupingIn = MIGEMO_OPINDEX_NEST_IN,
    GroupingOut = MIGEMO_OPINDEX_NEST_OUT,
    NewLine = MIGEMO_OPINDEX_NEWLINE,
    Or = MIGEMO_OPINDEX_OR,
  };

  Migemo() noexcept : migemo_(migemo_open(nullptr)) {}
  Migemo(const Migemo&) = delete;
  Migemo(Migemo&&) = default;

  Migemo& operator=(const Migemo&) = delete;
  Migemo& operator=(Migemo&&) = default;

  const uchar *GetOperator(OperatorType type) noexcept {
    return migemo_get_operator(migemo_.get(), static_cast<int>(type));
  }

  DataFileType Load(DataFileType dict_type, const char *filename) {
    int loaded_dict_type = migemo_load(
        migemo_.get(), static_cast<int>(dict_type), filename);
    if (loaded_dict_type == MIGEMO_DICTID_INVALID) {
      throw std::invalid_argument(std::string("Failed to load ") + filename);
    }
    return static_cast<DataFileType>(loaded_dict_type);
  }

  void LoadDictsInDirectory(const std::string& dirname) {
    std::string dirname_(dirname);
    if (dirname_.size() != 0 && dirname_[dirname_.size() - 1] != '/') {
      dirname_.append(1, '/');
    }

    const struct {
      DataFileType dict_type;
      const char *filename;
    } default_dict_names[] = {
      { DataFileType::Hankaku2Zenkaku, "han2zen.dat" },
      { DataFileType::Hiragana2Katakana, "hira2kata.dat" },
      { DataFileType::MigemoDict, "migemo-dict" },
      { DataFileType::Roman2Hiragana, "roma2hira.dat" },
      { DataFileType::Zenkaku2Hankaku, "zen2han.dat" },
    };
    for (const auto& default_dict_name : default_dict_names) {
      std::string filename(dirname_ + default_dict_name.filename);
      struct stat buf;
      if (stat(filename.c_str(), &buf) != 0) { continue; }
      Load(default_dict_name.dict_type, filename.c_str());
    }
  }

  bool Loaded() noexcept {
    return static_cast<bool>(migemo_is_enable(migemo_.get()));
  }

  template <typename DecoderType>
  void SetDecoder(DecoderType decoder) noexcept {
    migemo_setproc_char2int(migemo_.get(), decoder);
  }

  template <typename EncoderType>
  void SetEncoder(EncoderType encoder) noexcept {
    migemo_setproc_int2char(migemo_.get(), encoder);
  }

  bool SetOperator(OperatorType type, const uchar *op) noexcept {
    return migemo_set_operator(
        migemo_.get(), static_cast<int>(type), op) != 0;
  }

  std::unique_ptr<uchar, RegexpStringDeleter> Query(const uchar *query) {
    return std::unique_ptr<uchar, RegexpStringDeleter>(
        migemo_query(migemo_.get(), query));
  }

 private:
  class MigemoDeleter {
   public:
    void operator()(migemo *m) noexcept { migemo_close(m); }
  };

  class RegexpStringDeleter {
   public:
    // As of C/Migemo 1.3e, migemo_release() just calls free() with its 2nd
    // argument. So it's safe to pass |nullptr| instead of a pointer to living
    // migemo object.
    void operator()(uchar *regexp) noexcept {
      migemo_release(nullptr, regexp);
    }
  };

  std::unique_ptr<migemo, MigemoDeleter> migemo_;
};

}  // namespace cxxmigemo

#endif  // CXXMIGEMO_H_

