// Java Implementation (Illustrative - Focus on Structure, Not a Full File System)

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

class File {
    String name;
    String content; // For simplicity
    Map<String, String> permissions; // User/Role -> Permissions (e.g., "read", "write")

    public File(String name) {
        this.name = name;
        this.content = "";
        this.permissions = new HashMap<>();
    }

    public String getName() {
        return name;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public Map<String, String> getPermissions() {
        return permissions;
    }

    public void addPermission(String userOrRole, String permission) {
        this.permissions.put(userOrRole, permission);
    }
}

class Directory {
    String name;
    Map<String, Directory> subDirectories;
    Map<String, File> files;
    Map<String, String> permissions; // User/Role -> Permissions

    public Directory(String name) {
        this.name = name;
        this.subDirectories = new HashMap<>();
        this.files = new HashMap<>();
        this.permissions = new HashMap<>();
    }

    public String getName() {
        return name;
    }

    public Map<String, Directory> getSubDirectories() {
        return subDirectories;
    }

    public Map<String, File> getFiles() {
        return files;
    }

    public Map<String, String> getPermissions() {
        return permissions;
    }

    public void addSubDirectory(String name, Directory directory) {
        this.subDirectories.put(name, directory);
    }

    public void addFile(String name, File file) {
        this.files.put(name, file);
    }

    public Directory getSubDirectory(String name) {
        return this.subDirectories.get(name);
    }

    public File getFile(String name) {
        return this.files.get(name);
    }

    public void addPermission(String userOrRole, String permission) {
        this.permissions.put(userOrRole, permission);
    }
}

class FileSystem {
    Directory root;

    public FileSystem() {
        this.root = new Directory("/");
        initializeUniversityStructure();
        setDefaultPermissions();
    }

    private void initializeUniversityStructure() {
        Directory academics = new Directory("academics");
        Directory research = new Directory("research");
        Directory admin = new Directory("admin");
        Directory users = new Directory("users");
        Directory shared = new Directory("shared");

        root.addSubDirectory("academics", academics);
        root.addSubDirectory("research", research);
        root.addSubDirectory("admin", admin);
        root.addSubDirectory("users", users);
        root.addSubDirectory("shared", shared);

        Directory csCourses = new Directory("CS");
        Directory eeCourses = new Directory("EE");
        academics.getSubDirectory("courses").addSubDirectory("CS", csCourses);
        academics.getSubDirectory("courses").addSubDirectory("EE", eeCourses);
        csCourses.addSubDirectory("CS101", new Directory("CS101"));
        csCourses.addSubDirectory("CS201", new Directory("CS201"));

        Directory aiLab = new Directory("AI_Lab");
        research.getSubDirectory("projects").addSubDirectory("AI_Lab", aiLab);
        aiLab.addSubDirectory("ProjectAlpha", new Directory("ProjectAlpha"));

        admin.addSubDirectory("finance", new Directory("finance"));
        admin.addSubDirectory("hr", new Directory("hr"));
        admin.addSubDirectory("registrar", new Directory("registrar"));

        users.addSubDirectory("student123", new Directory("student123"));
        users.addSubDirectory("prof_smith", new Directory("prof_smith"));
        users.getSubDirectory("student123").addSubDirectory("courses", new Directory("courses"));
        users.getSubDirectory("prof_smith").addSubDirectory("lectures", new Directory("lectures"));

        shared.addSubDirectory("announcements", new Directory("announcements"));
        shared.addSubDirectory("public_documents", new Directory("public_documents"));
    }

    private void setDefaultPermissions() {
        // Example: Students have read/write in their own directory
        root.getSubDirectory("users").getSubDirectories().forEach((username, dir) -> {
            dir.addPermission(username, "read,write");
            if (dir.getSubDirectory("courses") != null) {
                dir.getSubDirectory("courses").addPermission(username, "read,write");
            }
            if (dir.getSubDirectory("lectures") != null) {
                dir.getSubDirectory("lectures").addPermission(username, "read,write");
            }
        });

        // Example: Faculty have read/write in their own directory and course directories
        Directory cs101 = root.getSubDirectory("academics").getSubDirectory("courses").getSubDirectory("CS").getSubDirectory("CS101");
        if (cs101 != null) {
            cs101.addPermission("prof_smith", "read,write");
        }
        Directory cs201 = root.getSubDirectory("academics").getSubDirectory("courses").getSubDirectory("CS").getSubDirectory("CS201");
        if (cs201 != null) {
            cs201.addPermission("prof_smith", "read,write");
        }

        // Example: Public documents are readable by all
        root.getSubDirectory("shared").getSubDirectory("public_documents").addPermission("all", "read");
    }

    public Directory getDirectory(String path) {
        String[] parts = path.split("/");
        Directory current = root;
        if (parts.length > 1 && parts[0].isEmpty()) { // Handle leading slash
            for (int i = 1; i < parts.length; i++) {
                if (current == null) {
                    return null;
                }
                current = current.getSubDirectory(parts[i]);
            }
            return current;
        }
        return null;
    }

    public File getFile(String path) {
        String[] parts = path.split("/");
        if (parts.length > 1 && parts[0].isEmpty()) {
            String fileName = parts[parts.length - 1];
            String directoryPath = path.substring(0, path.lastIndexOf("/"));
            Directory directory = getDirectory(directoryPath);
            if (directory != null) {
                return directory.getFile(fileName);
            }
        }
        return null;
    }

    public List<String> search(String query, String startPath) {
        List<String> results = new ArrayList<>();
        Directory startDir = getDirectory(startPath);
        if (startDir != null) {
            searchRecursive(startDir, startPath, query, results);
        }
        return results;
    }

    private void searchRecursive(Directory currentDir, String currentPath, String query, List<String> results) {
        for (File file : currentDir.getFiles().values()) {
            if (file.getName().contains(query)) {
                results.add(currentPath + "/" + file.getName());
            }
            // Basic content search (for illustration)
            if (file.getContent().contains(query)) {
                results.add(currentPath + "/" + file.getName() + " (content match)");
            }
        }
        for (Directory subDir : currentDir.getSubDirectories().values()) {
            searchRecursive(subDir, currentPath + "/" + subDir.getName(), query, results);
        }
    }

    public boolean checkPermission(String userOrRole, String path, String permissionNeeded) {
        Directory directory = getDirectory(path);
        if (directory != null && directory.getPermissions().containsKey(userOrRole)) {
            String permissions = directory.getPermissions().get(userOrRole);
            return permissions.contains(permissionNeeded);
        }
        File file = getFile(path);
        if (file != null && file.getPermissions().containsKey(userOrRole)) {
            String permissions = file.getPermissions().get(userOrRole);
            return permissions.contains(permissionNeeded);
        }
        return false; // Default deny
    }

    public static void main(String[] args) {
        FileSystem fs = new FileSystem();

        // Example interactions
        System.out.println("Directory structure initialized.");

        Directory cs101Dir = fs.getDirectory("/academics/courses/CS/CS101");
        if (cs101Dir != null) {
            cs101Dir.addFile("syllabus_fall2024.pdf", new File("syllabus_fall2024.pdf"));
            System.out.println("Syllabus added to /academics/courses/CS/CS101");
        }

        File assignment1 = new File("assignment1.docx");
        assignment1.setContent("This is the first assignment.");
        fs.getDirectory("/users/student123/courses/CS101").addFile("assignment1.docx", assignment1);
        System.out.println("Assignment added to /users/student123/courses/CS101");

        System.out.println("\nSearching for 'syllabus' in /academics:");
        List<String> searchResults = fs.search("syllabus", "/academics");
        searchResults.forEach(System.out::println);

        System.out.println("\nPermission check for student123 to read /academics/courses/CS/CS101/syllabus_fall2024.pdf:");
        System.out.println(fs.checkPermission("student123", "/academics/courses/CS/CS101", "read"));

        System.out.println("\nPermission check for prof_smith to write in /academics/courses/CS/CS101:");
        System.out.println(fs.checkPermission("prof_smith", "/academics/courses/CS/CS101", "write"));

        System.out.println("\nPermission check for student123 to write in /academics/courses/CS/CS101:");
        System.out.println(fs.checkPermission("student123", "/academics/courses/CS/CS101", "write"));
    }
}
