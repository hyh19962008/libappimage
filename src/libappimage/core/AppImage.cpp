// system
#include <iostream>
#include <algorithm>

// local
#include "appimage/core/AppImage.h"
#include "utils/MagicBytesChecker.h"
#include "utils/ElfFile.h"

namespace appimage {
    namespace core {

        /**
         * Implementation of the opaque pointer patter for the appimage class
         * see https://en.wikipedia.org/wiki/Opaque_pointer
         */
        class AppImage::Private {
        public:
            std::string path;
            FORMAT format = INVALID;

            static FORMAT getFormat(const std::string& path);
        };

        AppImage::AppImage(const std::string& path) : d(new Private()) {
            d->path = path;
            d->format = d->getFormat(path);

            if (d->format == INVALID)
                throw core::AppImageError("Unknown AppImage format");
        }

        const std::string& AppImage::getPath() const {
            return d->path;
        }

        FORMAT AppImage::getFormat() const {
            return d->format;
        }

        FORMAT AppImage::Private::getFormat(const std::string& path) {
            utils::magic_bytes_checker magicBytesChecker(path);
            if (magicBytesChecker.hasAppImageType1Signature())
                return TYPE_1;

            if (magicBytesChecker.hasAppImageType2Signature())
                return TYPE_2;

            if (magicBytesChecker.hasIso9660Signature() && magicBytesChecker.hasElfSignature()) {
                std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes."
                          << std::endl;
                return TYPE_1;
            }

            return INVALID;
        }

        AppImage::~AppImage() = default;

        FilesIterator AppImage::files() {
            return FilesIterator(d->path, d->format);
        }

        off_t AppImage::getPayloadOffset() const {
            utils::ElfFile elf(d->path);

            return elf.getSize();
        }

    }
}
