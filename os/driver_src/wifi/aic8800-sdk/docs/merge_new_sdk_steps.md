# How to merge new released version sdk

## Update docs

1. Use Markdown to export PDF if has updated in git: aic8800-sdk-docs
2. Update PDF and Content.md
3. Export a new Content.html

## Merge source code & libs

git apply ../patch/diff_sdk-v6.4.6.0_sdk-v6.4.7.0.patch --exclude=config/aic8800*/lib/*.a --exclude=config/aic8800*/res/*.bin --exclude=user/lib/aic8800*/*.a
(Copy lib files)
git add .
git commit -m "[host_wb:8d27fa33] update sdk version to v6.4.7.0"
git push origin

## New tag & branch
git tag sdk-v6.4.7.0 [COMMIT_ID]
git tag -a aic8800-sdk-6.4.7-base -m "new release base: v6.4.7" [COMMIT_ID]
git push origin --tag sdk-v6.4.7.0 aic8800-sdk-6.4.7-base
git branch aic8800-sdk-6.4.7 aic8800-sdk-6.4.7-base
git push origin aic8800-sdk-6.4.7:aic8800-sdk-6.4.7
