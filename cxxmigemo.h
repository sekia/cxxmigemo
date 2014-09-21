#ifndef CXXMIGEMO_H_
#define CXXMIGEMO_H_

#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "migemo.h"

namespace cxxmigemo {

/**
   Thin wrapper class of C/Migemo library for C++11.
 */
class Migemo {
 private:
  class RegexpStringDeleter;
  using uchar = unsigned char;

 public:
  /**
     Constants that specify kind of dictionary/mapping file to be Load()ed.
   */
  enum class DataFileType {
    Hankaku2Zenkaku = MIGEMO_DICTID_HAN2ZEN,
    Hiragana2Katakana = MIGEMO_DICTID_HIRA2KATA,
    MigemoDict = MIGEMO_DICTID_MIGEMO,
    Roman2Hiragana = MIGEMO_DICTID_ROMA2HIRA,
    Zenkaku2Hankaku = MIGEMO_DICTID_ZEN2HAN,
  };

  /**
     Constants that specify kind of RegExp fragment to be set by
     SetOperator().
   */
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

  /**
     Returns currently set RegExp fragment. By default setting Migemo
     generates PCRE expression.
     @param type Kind of fragment trying to set.
   */
  const uchar *GetOperator(OperatorType type) noexcept {
    return migemo_get_operator(migemo_.get(), static_cast<int>(type));
  }

  /**
     Loads a character dictionary/mapping file. If the file does not exist or
     cannot be read, an std::invalid_argument exception will be raised.
     @param dict_type Kind of file trying to load.
     @param filename A path to file to load.
   */
  DataFileType Load(DataFileType dict_type, const char *filename) {
    int loaded_dict_type = migemo_load(
        migemo_.get(), static_cast<int>(dict_type), filename);
    if (loaded_dict_type == MIGEMO_DICTID_INVALID) {
      throw std::invalid_argument(std::string("Failed to load ") + filename);
    }
    return static_cast<DataFileType>(loaded_dict_type);
  }

  /**
     Loads all the dictionary/mapping files having default names and at under
     specified directory. Non-existing files are silently ignored.
     Existing-but-unreadable files will be cause of std::invalid_argument
     exception.
     @param dirname A path to a directory including dictionary/mapping files.
   */
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

  /**
    @return true if a dictionary is loaded, false otherwise.
  */
  bool Loaded() noexcept {
    return static_cast<bool>(migemo_is_enable(migemo_.get()));
  }

  /**
     @param decoder Functor called when decoding dictionary/mapping files.
   */
  template <typename DecoderType>
  void SetDecoder(DecoderType decoder) noexcept {
    migemo_setproc_char2int(migemo_.get(), decoder);
  }

  /**
     @param encoder Functor called when encoding dictionary/mapping files.
   */
  template <typename EncoderType>
  void SetEncoder(EncoderType encoder) noexcept {
    migemo_setproc_int2char(migemo_.get(), encoder);
  }

  /**
     Sets RegExp fragment. This is useful when you want to generate pattern
     in non-PCRE dialect e.g. egrep or POSIX.
     @param type Kind of fragment trying to set.
     @param op RegExp fragment.
   */
  bool SetOperator(OperatorType type, const uchar *op) noexcept {
    return migemo_set_operator(
        migemo_.get(), static_cast<int>(type), op) != 0;
  }

  /**
     Generates a RegExp that matches possible Japanese conversions from
     SKK-style romaji input.
     @param query SKK-style romaji input.
     @return Pointer to assembled RegExp byte string.
   */
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
    /*
      As of C/Migemo 1.3e, migemo_release() just calls free() with its 2nd
      argument. So it's safe to pass |nullptr| instead of a pointer to living
      migemo object.
    */
    void operator()(uchar *regexp) noexcept {
      migemo_release(nullptr, regexp);
    }
  };

  std::unique_ptr<migemo, MigemoDeleter> migemo_;
};

}  // namespace cxxmigemo

#endif  // CXXMIGEMO_H_

