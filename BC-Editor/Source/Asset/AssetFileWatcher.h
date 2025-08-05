#pragma once

#include "Asset/AssetManagerAPI.h"

#include "Debug/Logging.h"

#include "Util/Hash.h"
#include "Util/Platform.h"

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <unordered_map>

namespace BC
{

    class AssetFileWatcher
    {

    private:

    #pragma region File Event Structs

        enum class FileEventType : uint8_t
        {
            New,
            Deleted,
            Renamed,
            Moved,
            Modified            
        };

        struct FileEvent
        {
            FileEventType type;
            virtual ~FileEvent() = default;
        };

        struct NewFileEvent : public FileEvent
        {
            std::filesystem::path file_path;
            NewFileEvent(const std::filesystem::path& path)
            {
                type = FileEventType::New;
                file_path = path;
            }
        };

        struct DeletedFileEvent : public FileEvent
        {
            std::filesystem::path file_path;
            DeletedFileEvent(const std::filesystem::path& path)
            {
                type = FileEventType::Deleted;
                file_path = path;
            }
        };

        struct RenamedFileEvent : public FileEvent
        {
            std::filesystem::path old_file_path;
            std::filesystem::path new_file_path;
            RenamedFileEvent(const std::filesystem::path& old_path, const std::filesystem::path& new_path)
            {
                type = FileEventType::Renamed;
                old_file_path = old_path;
                new_file_path = new_path;
            }
        };

        struct MovedFileEvent : public FileEvent
        {
            std::filesystem::path old_file_path;
            std::filesystem::path new_file_path;
            MovedFileEvent(const std::filesystem::path& old_path, const std::filesystem::path& new_path)
            {
                type = FileEventType::Moved;
                old_file_path = old_path;
                new_file_path = new_path;
            }
        };

        struct ModifiedFileEvent : public FileEvent
        {
            std::filesystem::path file_path;
            ModifiedFileEvent(const std::filesystem::path& path)
            {
                type = FileEventType::Modified;
                file_path = path;
            }
        };

        using FileEntryHash = uint64_t;

        struct FileEntry
        {
            std::filesystem::path path;
            std::filesystem::file_time_type last_write_time;
            uintmax_t size = 0;

            FileEntryHash content_hash = NULL_GUID;
        };
        
        using FileEntrySnapshot = std::unordered_map<std::string, FileEntry>;

    #pragma endregion

    public:

        AssetFileWatcher() = default;
        ~AssetFileWatcher() = default;

        AssetFileWatcher(const AssetFileWatcher& other) = delete;
        AssetFileWatcher(AssetFileWatcher&& other) = delete;
        AssetFileWatcher& operator=(const AssetFileWatcher& other) = delete;
        AssetFileWatcher& operator=(AssetFileWatcher&& other) = delete;

        bool IsWatching() const { return !m_RootDirectory.empty(); }
        auto& GetWatchDirectory() const { return m_RootDirectory; }

        void StartWatching(const std::filesystem::path& directory)
        {
            StopWatching();

            if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
            {
                BC_APP_ERROR("Cannot Start Watch - Directory Is Invalid: {}", directory.string());
                return;
            }
            
            m_RootDirectory = directory;
            if (!TakeSnapshot(m_PreviousSnapshot))
                StopWatching();
        }

        void StopWatching()
        {            
            m_RootDirectory.clear();
            m_PreviousSnapshot.clear();
        }

        void PollEvents()
        {
            if (!IsWatching() || !CompareSnapshots() || m_EventQueue.empty())
                return;

            for (const auto& event : m_EventQueue)
            {
                switch (event->type)
                {
                    case FileEventType::New:
                    {
                        BC_APP_INFO("File Added: {}", static_cast<NewFileEvent*>(event.get())->file_path.string());
                        break;
                    }
                    case FileEventType::Deleted:
                    {
                        BC_APP_INFO("File Deleted: {}", static_cast<DeletedFileEvent*>(event.get())->file_path.string());
                        break;
                    }
                    case FileEventType::Renamed:
                    {
                        BC_APP_INFO("File Renamed: {} -> {}", static_cast<RenamedFileEvent*>(event.get())->old_file_path.string(), static_cast<RenamedFileEvent*>(event.get())->new_file_path.string());
                        break;
                    }
                    case FileEventType::Moved:
                    {
                        BC_APP_INFO("File Moved: {} -> {}", static_cast<MovedFileEvent*>(event.get())->old_file_path.string(), static_cast<MovedFileEvent*>(event.get())->new_file_path.string());
                        break;
                    }
                    case FileEventType::Modified:
                    {
                        BC_APP_INFO("File Modified: {}", static_cast<ModifiedFileEvent*>(event.get())->file_path.string());
                        break;
                    }
                }
            }
            m_EventQueue.clear();
        }

    private:

        bool TakeSnapshot(FileEntrySnapshot& snapshot)
        {
            snapshot.clear();

            if (!IsWatching())
                return false;

            try
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(m_RootDirectory))
                {
                    if (entry.is_directory() || !entry.is_regular_file() || !AssetManager::IsExtensionSupported(entry.path().extension()))
                        continue;

                    auto path_str = entry.path().string();
                    auto last_write = entry.last_write_time();
                    auto size = entry.file_size();

                    FileEntryHash content_hash = 0;
                    auto prev_it = m_PreviousSnapshot.find(path_str);
                    if (prev_it != m_PreviousSnapshot.end() &&
                        prev_it->second.last_write_time == last_write &&
                        prev_it->second.size == size)
                    {
                        // Reuse previous hash
                        content_hash = prev_it->second.content_hash;
                    }
                    else
                    {
                        // Compute new hash
                        content_hash = Util::HashFileContents(entry.path()); // Hash File Contents
                    }

                    FileEntry file_entry;
                    file_entry.path = entry.path();
                    file_entry.last_write_time = last_write;
                    file_entry.size = size;
                    file_entry.content_hash = content_hash;

                    snapshot[path_str] = file_entry;
                }
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                BC_APP_ERROR("AssetFileWatcher::TakeSnapshot: Filesystem Error While Taking Snapshot: {}", e.what());
                snapshot.clear();
                return false;
            }

            return true;
        }

        bool CompareSnapshots()
        {
            FileEntrySnapshot new_snapshot{};
            if (!TakeSnapshot(new_snapshot))
                return false;

            FileEntrySnapshot new_file_entries{};
            FileEntrySnapshot deleted_file_entries{};
            FileEntrySnapshot unchanged_file_entries{};

            for (const auto& [file_path, file_entry] : m_PreviousSnapshot)
            {
                if (!new_snapshot.contains(file_path))
                    deleted_file_entries.emplace(file_path, file_entry);
            } 

            for (const auto& [file_path, file_entry] : new_snapshot)
            {
                if (!m_PreviousSnapshot.contains(file_path))
                    new_file_entries.emplace(file_path, file_entry);
                else
                    unchanged_file_entries.emplace(file_path, file_entry); // Stored most up to date file entry in this container as this will hold updated information on file entry, e.g., if size or write time or content hash changed
            }

            // 1. Find Renamed Files and Moved Files
            for (auto it = deleted_file_entries.begin(); it != deleted_file_entries.end(); )
            {
                const auto& [deleted_file_path, deleted_file_entry] = *it;

                bool resolved = false;

                // Check for Simple Rename
                for (const auto& [new_file_path, new_file_entry] : new_file_entries)
                {
                    // Check if same file
                    if (deleted_file_entry.path.parent_path() == new_file_entry.path.parent_path() &&
                        deleted_file_entry.content_hash == new_file_entry.content_hash &&
                        deleted_file_entry.size == new_file_entry.size &&
                        deleted_file_entry.last_write_time == new_file_entry.last_write_time)
                    {
                        // File was renamed
                        m_EventQueue.emplace_back(std::make_unique<RenamedFileEvent>(deleted_file_path, new_file_path));

                        // Remove from both maps to avoid further processing
                        new_file_entries.erase(new_file_path);
                        it = deleted_file_entries.erase(it);
                        resolved = true;

                        break; // No need to check further for this deleted file
                    }
                }

                if (resolved)
                {
                    continue; // Skip to next deleted file
                }

                // If not renamed, check for moved files
                for (const auto& [new_file_path, new_file_entry] : new_file_entries)
                {
                    // Check if same file but different path
                    if (deleted_file_entry.path.filename() == new_file_entry.path.filename() &&
                        deleted_file_entry.content_hash == new_file_entry.content_hash &&
                        deleted_file_entry.size == new_file_entry.size &&
                        deleted_file_entry.last_write_time == new_file_entry.last_write_time)
                    {
                        // File was moved
                        m_EventQueue.emplace_back(std::make_unique<MovedFileEvent>(deleted_file_path, new_file_path));

                        // Remove from both maps to avoid further processing
                        new_file_entries.erase(new_file_path);
                        it = deleted_file_entries.erase(it);
                        resolved = true;

                        break; // No need to check further for this deleted file
                    }
                }

                if (resolved)
                {
                    continue;
                }

                ++it;
            }

            for (const auto& [file_path, file_entry] : deleted_file_entries)
            {
                m_EventQueue.emplace_back(std::make_unique<DeletedFileEvent>(file_entry.path));
            }

            for (const auto& [file_path, file_entry] : new_file_entries)
            {
                m_EventQueue.emplace_back(std::make_unique<NewFileEvent>(file_entry.path));
            }

            // 2. Check for File Modifications
            for (const auto& [file_path, file_entry] : unchanged_file_entries)
            {
                auto& previous_file_entry = m_PreviousSnapshot[file_path];
                if (file_entry.content_hash != previous_file_entry.content_hash ||
                    file_entry.last_write_time != previous_file_entry.last_write_time ||
                    file_entry.size != previous_file_entry.size)
                {
                    m_EventQueue.emplace_back(std::make_unique<ModifiedFileEvent>(file_entry.path));
                }
            }

            // Update previous snapshot
            m_PreviousSnapshot = std::move(new_snapshot);

            return true;
        }

    private:
        
        std::filesystem::path m_RootDirectory{};
        FileEntrySnapshot m_PreviousSnapshot{};

        std::vector<std::unique_ptr<FileEvent>> m_EventQueue{};
    };

}