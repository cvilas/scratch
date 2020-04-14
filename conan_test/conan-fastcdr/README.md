For arrival:
- To create package `conan create . fastcdr/1.0.13@arrival/stable` (Use full package name with user/channel on Arrival server since it runs an old version of Conan)
- Before uploading package, set user: `conan user -r conan-tra chitrakaran@arrival.com`
- Upload package: `conan upload fastcdr/1.0.13@arrival/stable -r=conan-tra`
- On prompt, enter password for user `chitrakaran@arrival.com`

For my personal repo:
- To create package `conan create . fastcdr/1.0.13@vilas/stable`
- Before uploading package, set user: `conan user -r conan-vilas-stable cvilas`
- Upload package: `conan upload fastcdr/1.0.13@vilas/stable -r=conan-vilas-stable`
